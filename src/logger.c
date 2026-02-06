#include "logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define LOG_FILE_NAME "ripple_debug.log"

static FILE *log_file = NULL;

void log_init(void) {
    log_file = fopen(LOG_FILE_NAME, "w");
    if (!log_file) {
        fprintf(stderr, "Failed to open log file!");
    }
    log_debug("New session started");
}

void log_debug(const char *fmt, ...) {
    if (!log_file) {
        fprintf(stderr, "Attempted to log a debug statement with no open debug log file handle!\n");
        return;
    }

    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);

    fprintf(
        log_file,
        "%04d-%02d-%02d %02d:%02d:%02d ",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec
    );

    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);

    fputc('\n', log_file);
    fflush(log_file);
}

void log_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}