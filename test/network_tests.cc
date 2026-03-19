#include <gtest/gtest.h>

extern "C" {
    #include "../src/http_get_rss_xml.h"
    #include "../src/logger.h"
    #include "../src/queue.h"
    #include "../src/thread_pool.h"
    #include "../src/channel_manager.h"
    #include "../src/parser/xml_rss.h"
    #include "../src/utils.h"
}

TEST(http_get_tests, basic_fetch_insecure) {
    log_init();
    size_t size;
    http_response *response = send_http_get((char *)"http://file_server/basic_file.txt");
    ASSERT_NE(response, nullptr);
    ASSERT_STREQ(response->body, "This is a basic file");
    ASSERT_EQ(response->body_size, 20);
}

TEST(http_get_tests, basic_fetch_secure) {
    log_init();
    size_t size;
    http_response *response = send_http_get((char *)"https://file_server/basic_file.txt");
    ASSERT_NE(response, nullptr);
    ASSERT_STREQ(response->body, "This is a basic file");
    ASSERT_EQ(response->body_size, 20);
}

void *test_fetch_channel(void *channel_link, void *arg) {
    message_queue *final_queue = (message_queue *)arg;

    char *link = (char *)channel_link;
    
    http_response *response = send_http_get(link);
    if (!response) {
        return NULL;
    }

    rss_channel *new_channel = build_channel(response->body, response->body_size, link);
    free_http_response(response);

    if (!new_channel) {
        return NULL;
    }
    if (queue_enqueue((void *)new_channel, final_queue)) {
        free_channel(new_channel);
    }
    return NULL;
}

TEST(http_get_tests, stress_test) {
    log_init();
    const int num_fetches = 100;
    long t1 = current_time_ms();
    message_queue *final_queue = queue_init(num_fetches);
    thread_pool *pool = thread_pool_create(10, num_fetches, test_fetch_channel, final_queue); 

    for (size_t i = 0; i < num_fetches; i++) {
        thread_pool_add_work((void *)"https://file_server/basic.xml", pool);
    }

    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += 300;

    pthread_mutex_lock(&pool->info->mut);
    while (pool->info->working_count > 0 || !queue_empty(pool->info->work_queue)) {
        int rc = pthread_cond_timedwait(&pool->info->idle_cond, &pool->info->mut, &t);
        ASSERT_EQ(rc, 0);
    }
    long t2 = current_time_ms();
    printf("Took %ld ms to process %d requests\n", t2 - t1, num_fetches);
    ASSERT_EQ(final_queue->size, 100);
    while (!queue_empty(final_queue)) {
        rss_channel *chan = (rss_channel *)queue_dequeue(final_queue);
        EXPECT_NE(chan, nullptr);
    }
    
}