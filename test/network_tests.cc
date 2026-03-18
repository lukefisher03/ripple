#include <gtest/gtest.h>

extern "C" {
    #include "../src/http_get_rss_xml.h"
    #include "../src/logger.h"
}

TEST(http_get_tests, basic_fetch_insecure) {
    log_init();
    size_t size;
    http_response *response = send_http_get((char *)"http://file_server/basic_file.txt");
    ASSERT_STREQ(response->body, "This is a basic file");
    ASSERT_EQ(response->body_size, 20);
}

TEST(http_get_tests, basic_fetch_secure) {
    log_init();
    size_t size;
    http_response *response = send_http_get((char *)"https://file_server/basic_file.txt");
    ASSERT_STREQ(response->body, "This is a basic file");
    ASSERT_EQ(response->body_size, 20);
}