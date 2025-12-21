#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

void log_init(void);
void log_debug(const char *fmt, ...);
void log_close(void);

#endif