#include "utils.h"
#include "logger.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

char *file_to_string(const char *path, size_t *out_size) {
    char *str; // Buffer for storing the output file string content
    FILE *fptr;

    fptr = fopen(path, "rb");
    if (!fptr) {
        log_debug("Could not open file! %s\n", path);
        return NULL;
    }

    size_t capacity = 100000; // 100KB base size
    size_t buf_sz = 0;

    str = malloc(capacity);
    if (!str) {
        log_debug("Could not allocate space to hold file %s\n", path);
        fclose(fptr);
        return NULL;
    }

    size_t bytes_read;
    while (1) {
        if (buf_sz + CHUNK_SIZE >= MAX_FILE_SIZE - 1) {
            // Exit if the file goes over MAX_FILE_SIZE. (Minus 1 for null terminator)
            log_debug("File is too big. Must be smaller than %i bytes\n", MAX_FILE_SIZE);
            fclose(fptr);
            free(str);
            return NULL;
        }
        if (buf_sz + CHUNK_SIZE >= capacity - 1) {
            // Expand the array if the buffer reaches capacity.
            capacity *= 2;
            char *tmp = realloc(str, capacity);
            if (!tmp) {
                log_debug("Memory allocation error!\n");
                fclose(fptr);
                free(str);
                return NULL;
            }
            str = tmp;
        }

        bytes_read = fread(str + buf_sz, 1, CHUNK_SIZE, fptr); // Read 1 byte at a time.
        buf_sz += bytes_read;
        if (bytes_read < CHUNK_SIZE) {
            // Exit if the number of bytes read is less than the chunk size. This
            // means we're done.
            if (!feof(fptr)) {
                // If bytes read is less than the chunk size but no EOF was logged,
                // then we know that an error occurred.
                perror("read error");
                free(str);
                fclose(fptr);
                return NULL;
            }
            break;
        }
    }

    str[buf_sz] = '\0'; // Add a null terminating character so it's useable.

    fclose(fptr);
    *out_size = buf_sz;
    return str;
}

// ------ Convert RFC 822 timestamps to tm structs ------ //

bool rfc_822_to_utc_tm(char *timestamp, struct tm *tm) {
    log_debug("Attempting conversion of %s to a tm struct", timestamp);
    // All variants of the RFC 822 date format
    char *date_strings[] = {
        "%a, %d %b %Y %H:%M:%S", "%a, %d %b %Y %H:%M:%S GMT",
        "%a, %d %b %Y %H:%M",    "%a, %d %b %Y %H:%M GMT",
        "%d %b %Y %H:%M:%S",     "%d %b %Y %H:%M:%S GMT",
    };

    for (size_t i = 0; i < sizeof(date_strings) / sizeof(date_strings[0]);
         i++) {
        struct tm tmp_tm = {0};
        char *end = strptime(timestamp, date_strings[i], &tmp_tm);

        if (!end)
            continue;

        for (; *end == ' '; end++)
            ;

        if (*end == '\0') {
            log_debug("No timezone information given for timestamp: %s",
                      timestamp);
            return false;
        }
        if (*end == 'G') {
            log_debug("GMT time detected for %s", timestamp);
            *tm = tmp_tm;
            return true;
        }

        // Get the UTC offset
        // Determine offset direction
        int offset_str_len = strlen(end);
        if (offset_str_len < 5 || offset_str_len > 6) {
            log_debug("Given offset isn't valid: %s", end);
            return false;
        }

        int direction = *end++ == '+' ? 1 : -1;

        // Determine the hour offset
        int hour_offset = 0;
        char hours_str[3];
        memcpy(hours_str, end, 2);
        hours_str[2] = '\0';
        end += 2;

        errno = 0;
        hour_offset = strtol(hours_str, NULL, 10);
        if (errno == EINVAL || errno == ERANGE) {
            log_debug("Failed to parse UTC offset: '%s' - %s", hours_str,
                      strerror(errno));
            return false;
        }

        // Skip colon if present
        if (*end == ':')
            end++;

        // Determine the minutes offset
        int min_offset = 0;
        char mins_str[3];
        memcpy(mins_str, end, 2);
        mins_str[2] = '\0';
        
        errno = 0;
        min_offset = strtol(mins_str, NULL, 10);

        if (errno == EINVAL || errno == ERANGE) {
            log_debug("Failed to parse UTC offset: '%s' - %s", hours_str,
                      strerror(errno));
            return false;
        }

        tmp_tm.tm_hour += (direction * hour_offset);
        tmp_tm.tm_hour %= 24;
        tmp_tm.tm_min += (direction * min_offset);
        tmp_tm.tm_min %= 24;

        *tm = tmp_tm;
        return true;
    }

    return false;
}

int unix_time_to_formatted(int64_t unix_timestamp, char *str, size_t buf_len) {
    if (!str || buf_len < 21) {
        return 1;
    }

    struct tm tm_local = {0};
    // TODO: This conversion is dangerous on legacy systems, but it might not
    // really be that important. Just noting it.
    time_t converted_timestamp = (time_t)unix_timestamp;
    if (!localtime_r(&converted_timestamp, &tm_local)) {
        log_debug("Failed to convert UTC timestamp to local: %ld",
                  unix_timestamp);
        return 1;
    }

    if (!strftime(str, buf_len, "%b %e, %Y %I:%M %p", &tm_local)) {
        return 1;
    }

    return 0;
}