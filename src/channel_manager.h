#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

int refresh_channels(void);
int store_new_channels(char **links, size_t link_count);
int get_new_channel_links(const char *feeds_file, size_t length, generic_list *list);

#endif