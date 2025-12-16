#ifndef UI_UTILS
#define UI_UTILS

#include <stdlib.h>
#include <stdbool.h>

int display_menu(int y, 
                        const void *options, 
                        size_t option_size,
                        int option_count, 
                        int (*render_selection)(int, int, bool, const void *)
                       );

#endif