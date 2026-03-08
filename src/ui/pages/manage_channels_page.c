#include "handlers.h"
#include "../ui_utils.h"
#include "../../logger.h"
#include "../../channels_db/channel_db_api.h"
#include "../../list.h"
#include "../../termbox2/termbox2.h"
#include "../../utils.h"

static channel_column_widths col_widths = {
    .channel_name = 0.5,
    .article_count = 0.2,
    .last_updated = 0.3,
};

extern char *thick_divider;
extern char *thin_divider;
extern char *blank_line;

static int render_channel_list(renderer_params *params);

void manage_channels_page(app_state *app, local_state *state) {
    // struct tb_event ev;
    int width = tb_width();
    int y = 1;
    write_centered(y++, TB_GREEN, 0, "CHANNELS");

    generic_list *channel_list = list_init();
    get_channel_list(channel_list);
    channel_with_article_count **channels_with_extras = calloc(channel_list->count, sizeof(*channels_with_extras));

    if (!channels_with_extras) {
        log_debug("Failed to allocate memory for the channel list!");
    }

    for (size_t i = 0; i < channel_list->count; i++) {
        rss_channel *chan = channel_list->elements[i];
        int article_count = 0;
        get_channel_article_count(chan, &article_count);
        channel_with_article_count *new_chan = malloc(sizeof(*new_chan));
        new_chan->chan = chan;
        new_chan->article_count = article_count;
        channels_with_extras[i] = new_chan;
    }
    
    char *row = calloc(width + 1, sizeof(char));

    int offset = 0;
    offset += add_column(row + offset, col_widths.channel_name * width, "CHANNEL NAME");
    offset += add_column(row + offset, col_widths.article_count * width, "ARTICLE COUNT");
    offset += add_column(row + offset, col_widths.last_updated * width, "LAST UPDATED");
    memset(row + offset, ' ', width - offset);
    row[width] = '\0';

    y += 1;
    tb_printf(0, y++, TB_GREEN, 0, "%s", row);
    tb_printf(0, y++, TB_GREEN, 0, "%s", thick_divider);

    int nav_help_offset = 3;
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "ENTER", "view articles");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "b", "back");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "D", "delete");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "i", "import");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "E", "exit");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "v", "show/hide from main feed");

    menu_config config = {
        .y = y,
        .x = 0,
        .options = channels_with_extras,
        .option_size = sizeof(channel_with_article_count*),
        .option_count = channel_list->count,
        .renderer = &render_channel_list,
        .valid_input_list = "DbEiv",
        .valid_input_count = 5,
        .row = row,
        .row_length = width,
        .default_selection = state->channels_state.default_selection ? state->channels_state.default_selection : 0,
    };

    // This gets overwritten if there's articles to display
    write_centered(y + 2, TB_GREEN, 0, "no channels, start by importing some channels");

    menu_result result = display_menu(config);
    free(row);

    rss_channel *selected_channel = NULL;
    char selected_channel_title[512] = "";
    int selected_channel_id = -1;
   
    if (!list_is_empty(channel_list)) {
        selected_channel = channel_list->elements[result.selection];
        snprintf(selected_channel_title, 512, "%s", selected_channel->title);
        selected_channel_id = selected_channel->id;
    }

    for (size_t i = 0; i < channel_list->count; i++) {
        free_channel(channel_list->elements[i]);
        free(channels_with_extras[i]);
    }

    list_free(channel_list);

    free(channels_with_extras);

    tb_present();
    if (result.ev.key == TB_KEY_ENTER && selected_channel) {
        navigate(CHANNEL_PAGE, app, (local_state){
            .channel_state = {
                .channel_id = selected_channel_id,
            },
        });
    }

    switch (result.ev.ch) {
        case 'D': {
            char msg[CONFIRMATION_MSG_SIZE];
            size_t chars_written = 0;
            char *options[2]; 
            if (selected_channel_id > -1) {
                chars_written = snprintf(msg, CONFIRMATION_MSG_SIZE, "Are you sure you wish to delete channel, %s?", selected_channel_title);
                options[0] = "yes";
                options[1] = "no";
            } else {
                chars_written = snprintf(msg, CONFIRMATION_MSG_SIZE, "You must import a channel before you can delete a channel.");
                options[0] = "back";
                options[1] = "exit";
            }

            if (chars_written >= CONFIRMATION_MSG_SIZE) {
                memcpy(msg + CONFIRMATION_MSG_SIZE - 4, "...", 3);
                msg[CONFIRMATION_MSG_SIZE - 1] = '\0';
            }
            menu_result result = display_confirmation_menu(msg, options, 2);
            
            if (result.selection == 0 && selected_channel_id > -1) {
                delete_channel(selected_channel_id);
            } 
                // TODO: Should probably do some error handling here
            
            if (result.selection == 1 && !selected_channel) {
                navigate(EXIT_PAGE, app, (local_state){});
            }
            break;
        }
        case 'b':
            navigate(MAIN_PAGE, app, (local_state){});
            break;
        case 'E':
            navigate(EXIT_PAGE, app, (local_state){});
            break;
        case 'i':
            navigate(IMPORT_PAGE, app, (local_state){});
            break;
        case 'v':
            if (toggle_channel_visibility(selected_channel_id) != 0) {
                log_debug("Failed to toggle visibility!");
            }
            navigate(CHANNELS_PAGE, app, (local_state){
                .channels_state = {
                    .default_selection = result.selection 
                }
            });
            break;
        default:
            break;
    }
}

static int render_channel_list(renderer_params *params) {
    const channel_with_article_count *chan_with_extras = *(channel_with_article_count**)params->option;
    const rss_channel *chan = chan_with_extras->chan;

    char *row = params->config->row;
    int row_length = params->config->row_length;

    int new_y = params->start_y;
    uintattr_t bg = params->selected ? SELECTED_BG_COLOR : 0;
    uintattr_t text_color = params->selected ? SELECTED_FG_COLOR : TB_GREEN;
    int offset = 0;

    char title[256];
    if (chan->shown) {
        snprintf(title, 256, "%s", chan->title);
    } else {
        snprintf(title, 256, "* %s", chan->title);
    }
    offset += add_column(row + offset, col_widths.channel_name * row_length, title);
    char article_count_str[28];
    snprintf(article_count_str, 28, "%d", chan_with_extras->article_count);
    offset += add_column(row + offset, col_widths.article_count * row_length, article_count_str);
    char formatted_date[128] = "";
    unix_time_to_formatted(chan->last_updated, formatted_date, 128);
    offset += add_column(row + offset, col_widths.last_updated * row_length, formatted_date);

    memset(row + offset, ' ', row_length - offset);
    row[row_length] = '\0';

    tb_printf(0, new_y++, TB_GREEN, bg, "%s", blank_line);
    tb_printf(0, new_y++, text_color, bg, "%s", row);
    tb_printf(0, new_y++, TB_GREEN, bg, "%s", blank_line);
    tb_printf(0, new_y++, TB_GREEN, 0, "%s", thin_divider);
    return new_y - params->start_y;
}
