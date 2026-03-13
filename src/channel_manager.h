#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

#include "list.h"
#include <stdlib.h>

#define REFRESH_PERIOD_HOURS 10

int refresh_channels(void);
int get_new_channel_links(const char *feeds_file, size_t length, generic_list *list);
int create_thread_pools(void);
int fetch_parse_tp_enqueue(char *link);
int fetch_parse_tp_busy(void);

#endif