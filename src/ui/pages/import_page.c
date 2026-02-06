#include "handlers.h"
#include "../ui_utils.h"
#include "../../utils.h"
#include "../../channel_manager.h"
#include "../../logger.h"

extern char *thin_divider;


static int show_banner(int x, int y, int width);
static int render_link_list(renderer_params *params);

void import_page(app_state *app, local_state *state) {
    int y = 1;
    write_centered(y++, TB_GREEN, 0, "Would you like to import these channels?");
    y++;
    generic_list *links = list_init();

    size_t size = 0;
    char *new_channels_file = file_to_string(app->init_state.new_channel_links_file_path, &size);

    get_new_channel_links(new_channels_file, size, links);

    int padding = tb_width() * 0.2;
    int width = tb_width() * 0.6;
    y += show_banner(padding, y, width);
    char *row = calloc(tb_width(), sizeof(char));
    menu_config config = {
        .y = y,
        .x = padding,
        .option_size = sizeof(char *),
        .option_count = links->count,
        .options = links->elements,
        .valid_input_list = "bEh",
        .valid_input_count = 4,
        .renderer = render_link_list,
        .row = row,
        .row_length = width,
    };

    int nav_offset = 0;
    nav_offset += print_navigation_help(padding + nav_offset, tb_height() - 2, "ENTER", "import");
    nav_offset += print_navigation_help(padding + nav_offset, tb_height() - 2, "b", "back");
    nav_offset += print_navigation_help(padding + nav_offset, tb_height() - 2, "h", "home");

    menu_result result = display_menu(config);
    
    free(row);

    switch (result.ev.ch) {
    case 'b':
        navigate(CHANNELS_PAGE, app, (local_state){});
        break;
    case 'h':
        navigate(MAIN_PAGE, app, (local_state){});
        break;
    default:
        break;
    }

    if (result.ev.key == TB_KEY_ENTER) {
        tb_clear();
        write_centered(tb_height() / 2 - 1, TB_GREEN, 0, "Importing channels, please wait");
        tb_present();
        if (store_new_channels((char**)links->elements, links->count) == 0) {
            navigate(CHANNELS_PAGE, app, (local_state){});
        }
    }

    for (size_t i = 0; i < links->count; i++) {
        free(links->elements[i]);
    }
    free(links);
}

static int render_link_list(renderer_params *params) {
    char *link = *(char**)params->option;
    char *row = params->config->row;
    int row_len = params->config->row_length;

    int offset = 0;
    char idx_str[64];
    if (snprintf(idx_str, 64, "%d", params->idx) >= 64) {
        log_debug("Too many rows, index number too large!");
        // TODO: Handle this error
    }
    offset += add_column(row + offset, row_len * 0.2, idx_str);
    offset += add_column(row + offset, row_len * 0.8, link);
    memset(row + offset, ' ', row_len - offset);
    row[row_len] = '\0';

    uintattr_t bg = params->selected ? TB_BLACK : 0;
    int new_y = params->start_y;
    tb_printf(params->config->x, new_y++, TB_GREEN, bg, "%s", row);

    return new_y - params->start_y;
}

static int show_banner(int x, int y, int width) {
    char *row = calloc(width + 1, sizeof(char));
    if (!row) {
        log_debug("Memory allocation failed");
        abort();
    }
    int offset = 0;

    offset += add_column(row + offset, width * 0.2, "#");
    offset += add_column(row + offset, width * 0.8, "CHANNEL LINK");
    memset(row + offset, ' ', width - offset);
    row[width] = '\0';
    
    int new_y = y;
    tb_printf(x, new_y++, TB_GREEN, 0, "%s", row);
    memset(row, '-', width);
    row[width] = '\0';
    tb_printf(x, new_y++, TB_GREEN, 0, "%s", row);
    
    return new_y - y;
}

