#ifndef LOGGER_H
#define LOGGER_H

void log_init(void);
void log_debug(const char *fmt, ...);
void log_close(void);

#endif