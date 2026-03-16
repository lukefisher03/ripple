#include <gtest/gtest.h>

extern "C" {
    #include "../src/queue.h"
}

TEST(queue_tests, enqueue) {
    message_queue *q = queue_init(5);

    int x = 5;
    queue_enqueue(&x, q);
    
    ASSERT_EQ(q->items[q->front], &x);
    ASSERT_EQ(q->size, 1);
    ASSERT_EQ(q->back, q->front + 1);
}

TEST(queue_tests, dequeue) {
    message_queue *q = queue_init(5);

    int x = 5;
    queue_enqueue(&x, q);
    
    ASSERT_EQ(queue_dequeue(q), &x);
    ASSERT_EQ(q->size, 0);
    ASSERT_EQ(q->back, q->front);
}

TEST(queue_tests, queue_filled) {
    message_queue *q = queue_init(5);

    int nums[5];

    for (size_t i = 0; i < 5; i++) {
        nums[i] = i + 1;
        queue_enqueue(&nums[i], q);
    }

    int final_num = 1;
    int res = queue_enqueue(&final_num, q);
    
    ASSERT_EQ(res, 1);
    ASSERT_EQ(q->size, 5);
    ASSERT_EQ(queue_dequeue(q), &nums[0]);
}