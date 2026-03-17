#include <gtest/gtest.h>

extern "C" {
    #include "../src/utils.h"
}

TEST(utility_tests, read_file) {
    size_t size;
    char expected_contents[] = "This is a basic file";

    char *file_contents = file_to_string("test/test_files/basic_file.txt", &size);
    ASSERT_STREQ(file_contents, expected_contents);
    free(file_contents);
}

TEST(utility_tests, read_nonexistent_file) {
    size_t size;

    char *file_contents = file_to_string("nonexistent/path/here", &size);
    ASSERT_EQ(file_contents, nullptr);
}

TEST(utility_tests, rfc_822_conversion_valid_formats) {
    const char *timestamps[] = {
        "Mon, 16 Mar 2026 18:12:32 +0000",
        "Mon, 16 Mar 2026 18:12:32 -0000",
        "Mon, 16 Mar 2026 18:12:32 +1200",
        "Mon, 16 Mar 2026 18:12:32 -0300",
        "Mon, 16 Mar 2026 18:12:32 -0353",
    };

    for (size_t i = 0; i < sizeof(timestamps) / sizeof(char *); i++) {
        struct tm tmp_tm;
        ASSERT_EQ(rfc_822_to_utc_tm((char *)timestamps[i], &tmp_tm), 0);
    }
}


TEST(utility_tests, rfc_822_conversion_invalid_formats) {
    const char *timestamps[] = {
        "Mon, 16 Mar 2026 18:12:32",
        "Mon, 16 Mar 2026 18:12:32 -35",
        "Mon, 16 Mar 2026 18:12:32 1200",
        "Mon, 16 Mar 2026 18:12:32 0",
        "Mon, 16 Mar 202",
    };

    for (size_t i = 0; i < sizeof(timestamps) / sizeof(char *); i++) {
        struct tm tmp_tm;
        ASSERT_EQ(rfc_822_to_utc_tm((char *)timestamps[i], &tmp_tm), 1);
    }
}