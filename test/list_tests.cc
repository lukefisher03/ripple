#include <gtest/gtest.h>

extern "C" {
    #include "../src/list.h"
}

TEST(list_tests, basic_append) {
    generic_list *l = list_init();

    int x = 5;

    ASSERT_EQ(list_append(l, &x), 0);
    ASSERT_EQ(l->elements[0], &x);
}

TEST(list_tests, large_append) {
    // This will force the list to re-allocate
    generic_list *l = list_init();

    int nums[LIST_DEFAULT_CAPACITY * 5];

    for (size_t i = 0; i < LIST_DEFAULT_CAPACITY * 5; i++) {
        nums[i] = i + 1;
        ASSERT_EQ(list_append(l, &nums[i]), 0);
    }

    for (size_t i = 0; i < LIST_DEFAULT_CAPACITY * 5; i++) {
        ASSERT_EQ(l->elements[i], &nums[i]);
    }

    ASSERT_EQ(l->count, LIST_DEFAULT_CAPACITY * 5);
}

TEST(list_tests, pop) {
    generic_list *l = list_init();

    int nums[3];

    for (size_t i = 0; i < 3; i++) {
        nums[i] = i + 1;
        ASSERT_EQ(list_append(l, &nums[i]), 0);
    }
    
    for (size_t i = 0; i < 3; i++) {
        ASSERT_EQ(list_pop(l), &nums[2 - i]);
    }

    ASSERT_EQ(l->count, 0);
}

TEST(bounded_list_tests, append) {
    bounded_list *l = bounded_list_init(5);
    int nums[5];
    for (size_t i = 0; i < 5; i++) {
        nums[i] = (int)i + 1;
        bounded_list_append(l, &nums[i]);
    }

    int end = 5;
    while (!bounded_list_empty(l)) {
        int *n = (int *)bounded_list_pop(l);
        EXPECT_EQ(*n, end--);
    }
}

TEST(bounded_list_tests, append_overwrite) {
    bounded_list *l = bounded_list_init(3);
    int nums[5];
    for (size_t i = 0; i < 5; i++) {
        nums[i] = (int)i + 1;
        bounded_list_append(l, &nums[i]);
    }

    int end = 5;
    while (!bounded_list_empty(l)) {
        int *n = (int *)bounded_list_pop(l);
        EXPECT_EQ(*n, end--);
    }
}