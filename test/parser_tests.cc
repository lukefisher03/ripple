#include <gtest/gtest.h>

extern "C" {
    #include "../src/parser/xml_rss.h"
    #include "../src/utils.h"
    #include <string.h>
}

TEST(parser_tests, basic_accumulate_text) {
    char content[] = "This is some text content";
    char full_str[1024];
    snprintf(full_str, 1024, "%s</item>", content);

    rss_node *node = text_node_init();
    ASSERT_EQ(_accumulate_text(full_str, strlen(full_str) + 1, node), 25);
    ASSERT_STREQ(node->text, content);
}

TEST(parser_tests, cdata_accumulate_text) {
    char content[] = "This is some <i>text</i> <br> <p>content</p>";
    char full_str[1024];
    snprintf(full_str, 1024, "<![CDATA[%s]]>", content);

    rss_node *node = text_node_init();
    ASSERT_EQ(_accumulate_text(full_str, strlen(full_str) + 1, node), 56);
    ASSERT_STREQ(node->text, content);
}

TEST(parser_tests, entity_replacement_accumulate_text) {
    char raw_content[] = "&lt;test&gt; this is a test &lt;/test&gt;";
    char expected_content[] = "<test> this is a test </test>";
    char full_str[1024];
    snprintf(full_str, 1024, "%s</item>", raw_content);

    rss_node *node = text_node_init();
    ASSERT_GT(_accumulate_text(full_str, strlen(full_str) + 1, node), 0);
    ASSERT_STREQ(node->text, expected_content);
}

TEST(parser_tests, skip_comments) {
    char content[] = "<!-- this is a test comment \n\n \t -- -- - this is more comment text -->";
    ASSERT_EQ(_skip_comment(content, strlen(content) + 1), 70);
}

TEST(parser_tests, basic_read_tag_names) {
    xml_tag open_tag;
    char opening_tag[] = "<test_tag>";
    _read_tag(opening_tag, strlen(opening_tag) + 1, &open_tag);
    EXPECT_STREQ(open_tag.name, "test_tag");
    EXPECT_EQ(open_tag.tag_type, TAG_OPEN);

    xml_tag close_tag;
    char closing_tag[] = "</test_tag>";
    _read_tag(closing_tag, strlen(closing_tag) + 1, &close_tag);
    EXPECT_STREQ(close_tag.name, "test_tag");
    EXPECT_EQ(close_tag.tag_type, TAG_CLOSE);

    xml_tag self_close_tag;
    char self_closing_tag[] = "<test_tag/>";
    _read_tag(self_closing_tag, strlen(self_closing_tag) + 1, &self_close_tag);
    EXPECT_STREQ(self_close_tag.name, "test_tag");
    EXPECT_EQ(self_close_tag.tag_type, TAG_SELF_CLOSE);
}

TEST(parser_tests, test_parse_tree) {
    char xml[] = "<rss version='2.0'> \
    <channel> \
        <title>Basic Feed</title> \
        <link>https://basic-feed.com/</link> \
        <description>A basic test RSS feed</description> \
        <item> \
            <title>First Article</title> \
            <author>Test Author</author> \
            <link>https://engineering.fb.com/2026/03/02/data-infrastructure/investing-in-infrastructure-metas-renewed-commitment-to-jemalloc/</link> \
            <pubDate>Mon, 16 Mar 2026 18:12:32 +0000</pubDate> \
            <comments>https://news.ycombinator.com/item?id=47402640</comments> \
            <description>This is a description</description> \
        </item> \
    </channel> \
    </rss>";
    rss_node *tree = _construct_parse_tree(xml, strlen(xml) + 1);
    ASSERT_NE(tree, nullptr);

    rss_channel *channel = channel_init();
    ASSERT_EQ(_build_channel_from_parse_tree(channel, tree), 0);

    EXPECT_STREQ(channel->description, "A basic test RSS feed");
    EXPECT_STREQ(channel->title, "Basic Feed");
    EXPECT_STREQ(channel->link, "https://basic-feed.com/");
    ASSERT_EQ(channel->items->count, 1);

    rss_item *item = (rss_item *)channel->items->elements[0];
    EXPECT_STREQ(item->title, "First Article");
    EXPECT_STREQ(item->link, "https://engineering.fb.com/2026/03/02/data-infrastructure/investing-in-infrastructure-metas-renewed-commitment-to-jemalloc/");
    EXPECT_STREQ(item->description, "This is a description");
    EXPECT_STREQ(item->author, "Test Author");
    EXPECT_EQ(item->unix_timestamp, 1773684752);
}

TEST(parser_tests, parse_malformed_feed_0) {
    size_t size;
    char *xml = file_to_string("test/test_files/malformed_feed_0.xml", &size); 
    rss_channel *chan = build_channel(xml, strlen(xml) + 1, (char *)"testlink");
    ASSERT_EQ(chan, nullptr);
}

TEST(parser_tests, parse_malformed_feed_1) {
    size_t size;
    char *xml = file_to_string("test/test_files/malformed_feed_1.xml", &size); 
    rss_channel *chan = build_channel(xml, strlen(xml) + 1, (char *)"testlink");
    ASSERT_EQ(chan, nullptr);
}