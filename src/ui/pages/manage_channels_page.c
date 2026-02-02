#include "handlers.h"
#include "../ui_utils.h"
#include "../../logger.h"
#include "../../channels_db/channel_db_api.h"
#include "../../list.h"
#include "../../termbox2/termbox2.h"
#include "../../utils.h"

channel_column_widths col_widths;

int screen_width = 0;
extern char *thick_divider;
extern char *thin_divider;
extern char *blank_line;
char *row; 
int selection = 0;

static void set_channels_page_column_widths(channel_column_widths *widths);
static int render_channel_list(int x, int y, bool selected, const void *chanel);

void manage_channels_page(app_state *app, local_state *state) {
    // struct tb_event ev;
    screen_width = tb_width();
    set_channels_page_column_widths(&col_widths);
    int y = 1;
    write_centered(y++, TB_GREEN, 0, "MODIFY CHANNELS");

    generic_list *channel_list = list_init();
    get_channel_list(channel_list);
    channel_with_extras **channels_with_extras = calloc(channel_list->count, sizeof(*channels_with_extras));

    if (!channels_with_extras) {
        log_debug("Failed to allocate memory for the channel list!");
    }

    for (size_t i = 0; i < channel_list->count; i++) {
        rss_channel *chan = channel_list->elements[i];
        int article_count = 0;
        get_channel_article_count(chan, &article_count);
        log_debug("Lookint at element %d %d", i, channel_list->count);
        channel_with_extras *new_chan = malloc(sizeof(*new_chan));
        new_chan->chan = chan;
        new_chan->article_count = article_count;
        channels_with_extras[i] = new_chan;
    }
    
    row = calloc(screen_width + 1, sizeof(char));

    int offset = 0;
    offset += add_column(row + offset, col_widths.channel_name, "CHANNEL NAME");
    offset += add_column(row + offset, col_widths.article_count, "ARTICLE COUNT");
    offset += add_column(row + offset, col_widths.last_updated, "LAST UPDATED");
    memset(row, ' ', screen_width - offset);
    row[screen_width] = '\0';

    y += 1;
    tb_printf(0, y++, TB_GREEN, 0, row);
    tb_printf(0, y++, TB_GREEN, 0, thick_divider);

    menu_result result = display_menu(y, channels_with_extras, sizeof(channel_with_extras *), channel_list->count, &render_channel_list);
    tb_present();

    switch (result.ev.ch)
    {
    case 'd': {
        rss_channel *selected_channel = channel_list->elements[result.selection];
        char msg[CONFIRMATION_MSG_SIZE];
        size_t chars_written = snprintf(msg, CONFIRMATION_MSG_SIZE, "Are you sure you wish to delete channel, %s?", selected_channel->title);
        if (chars_written >= CONFIRMATION_MSG_SIZE) {
            memcpy(msg + CONFIRMATION_MSG_SIZE - 4, "...", 3);
            msg[CONFIRMATION_MSG_SIZE - 1] = '\0';
            log_debug("Went over!");
        }
        char *options[] = {"no", "yes"};
        menu_result result = display_confirmation_menu(msg, options, 2);
        
        if (result.selection == 1) {
            int result = delete_channel(selected_channel);
            if (result != 0) {
                log_debug("Failed to delete channel %s", selected_channel->title);
            }
        }
        break;
    }
    default:
        navigate(MAIN_PAGE, app, (local_state){});
        break;
    }

}

static int render_channel_list(int x, int y, bool selected, const void *channel) {
    (void) x;
    int screen_width = tb_width();
    const channel_with_extras *chan_with_extras = *(channel_with_extras**)channel;
    const rss_channel *chan = chan_with_extras->chan;

    int new_y = y;
    uintattr_t bg = selected ? TB_BLACK : 0;
    int offset = 0;
    offset += add_column(row + offset, col_widths.channel_name, chan->title);
    char article_count_str[28];
    snprintf(article_count_str, 28, "%d", chan_with_extras->article_count);
    offset += add_column(row + offset, col_widths.article_count, article_count_str);
    char formatted_date[128] = "";
    unix_time_to_formatted(chan->last_updated, formatted_date, 128);
    offset += add_column(row + offset, col_widths.last_updated, formatted_date);
    memset(row + offset, ' ', screen_width - offset);
    row[screen_width] = '\0';

    tb_printf(x, new_y++, TB_GREEN, bg, blank_line);
    tb_printf(x, new_y++, TB_GREEN, bg, row);
    tb_printf(x, new_y++, TB_GREEN, bg, blank_line);
    tb_printf(x, new_y++, TB_GREEN, 0, thin_divider);
    return new_y - y;
}

static void set_channels_page_column_widths(channel_column_widths *widths) {
    int SCREEN_WIDTH = tb_width();
    widths->channel_name = SCREEN_WIDTH * (0.5);
    widths->article_count = SCREEN_WIDTH * (0.2);
    widths->last_updated = SCREEN_WIDTH * (0.3);
}