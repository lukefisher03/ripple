#include <gtest/gtest.h>

extern "C" {
    #include "../src/queue.h"
}

TEST(HelloTest, BasicAssertions) {
    EXPECT_STRNE("hello", "world");
    EXPECT_EQ(7*6, 42);
}

TEST(queue_test_1, BasicAssertions) {
    message_queue *q = queue_init(10);
    int x = 5;
    queue_enqueue(&x, q);
    EXPECT_EQ(queue_dequeue(q), &x);
}