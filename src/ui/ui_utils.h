#ifndef UI_UTILS
#define UI_UTILS

#include "../termbox2/termbox2.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define SELECTED_BG_COLOR TB_GREEN 
#define SELECTED_FG_COLOR TB_WHITE 

typedef struct renderer_params renderer_params;
typedef int (*option_renderer)(renderer_params *params);

typedef struct {
    int                 x;
    int                 y;
    const void          *options;
    size_t              option_size;
    int                 option_count;
    int                 option_height;
    option_renderer     renderer;
    char                *valid_input_list;
    int                 valid_input_count;
    char                *row;
    int                 row_length;
    int                 default_selection;
} menu_config;

struct renderer_params{
    bool            selected;
    int             idx;
    const void      *option;
    int             start_y;
    menu_config     *config;
};


typedef struct {
    int             selection;
    struct tb_event ev;
} menu_result;


menu_result display_menu(menu_config config);

menu_result display_basic_menu(int y, 
                       const void *options, 
                       int option_count
                       );

menu_result display_confirmation_menu(const char *msg, char **options, int option_count);

int write_centered(int y, uintattr_t fg, uintattr_t bg, const char *text);
int print_logo(int x, int y, uintattr_t fg, uintattr_t bg);
size_t add_column(char *row, int col_width, const char *col_str);
void reset_dividers(void);
int print_navigation_help(int x, int y, char *key, char *instruction);
#endif