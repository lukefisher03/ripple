#ifndef UI_UTILS
#define UI_UTILS

#include "../termbox2/termbox2.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct app_state app_state;
typedef int (*option_renderer)(int, int, bool, const void *);

typedef struct menu_result {
    int             selection;
    struct tb_event ev;
} menu_result;

menu_result display_menu(int y, 
                 const void *options, 
                 size_t option_size,
                 int option_count, 
                 option_renderer render_selection
                );

menu_result display_basic_menu(int y, 
                       const void *options, 
                       int option_count
                       );

menu_result display_confirmation_menu(const char *msg, char **options, int option_count);

int write_centered(int y, uintattr_t fg, uintattr_t bg, const char *text);
int display_logo(int x, int y, uintattr_t fg, uintattr_t bg);
size_t add_column(char *row, int col_width, const char *col_str);
void reset_dividers(void);
#endif