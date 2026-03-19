#include "logger.h"

#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include <time.h>

#define LOG_FILE_NAME "config/ripple_debug.log"

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_init(void) {
    log_file = fopen(LOG_FILE_NAME, "w");
    if (!log_file) {
        fprintf(stderr, "Failed to open log file!");
    }
    log_debug("New session started");
}

static void _log_debug(const char *fmt, va_list args) {
    if (!log_file) return;

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
    vfprintf(log_file, fmt, args);
    fputc('\n', log_file);
    fflush(log_file);
}

void log_debug(const char *fmt, ...) {
    pthread_mutex_lock(&log_mutex);

    va_list args;
    va_start(args, fmt);

    _log_debug(fmt, args);

    va_end(args);

    pthread_mutex_unlock(&log_mutex);
}

void log_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}