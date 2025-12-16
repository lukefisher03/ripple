#ifndef UI_UTILS
#define UI_UTILS

#include "../termbox2/termbox2.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

int display_menu(int y, 
                        const void *options, 
                        size_t option_size,
                        int option_count, 
                        int (*render_selection)(int, int, bool, const void *)
                       );

int write_centered(int y, uintattr_t fg, uintattr_t bg, const char *text);
int display_logo(int x, int y, uintattr_t fg, uintattr_t bg);

#endif