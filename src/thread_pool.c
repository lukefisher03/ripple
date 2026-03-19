#include "thread_pool.h"
#include "queue.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>

void busy_wait_long(void) {
    for (size_t i = 0; i < 2000000000; i++);
} 

void busy_wait_short(void) {
    for (size_t i = 0; i < 50000000; i++);
}

int thread_pool_add_work(void *work, thread_pool *pool) {
    if (!pool) return 1;
    int res;
    pthread_mutex_lock(&pool->info->mut);
    res = queue_enqueue(work, pool->info->work_queue);
    pthread_cond_signal(&pool->info->work_cond);
    pthread_mutex_unlock(&pool->info->mut);
    return res;
}

int thread_pool_busy(thread_pool *pool) {
    pthread_mutex_lock(&pool->info->mut);
    int busy = pool->info->working_count > 0;
    pthread_mutex_unlock(&pool->info->mut);
    return busy;
}

void *do_work(void *args) {
    thread_info *thread_args = (thread_info *)args;

    while (1) {
        pthread_mutex_lock(&thread_args->mut);
        while (thread_args->work_queue->size == 0 && !thread_args->stop_work) {
            pthread_cond_wait(&thread_args->work_cond, &thread_args->mut);
        }

        if (thread_args->stop_work) {
            pthread_mutex_unlock(&thread_args->mut);
            break;
        }

        thread_args->working_count++;
        
        void *message = queue_dequeue(thread_args->work_queue);
        pthread_mutex_unlock(&thread_args->mut);
        if (!message) {
            continue;
        }
        thread_args->func(message, thread_args->arg);
        pthread_mutex_lock(&thread_args->mut);
        if (--thread_args->working_count == 0 && queue_empty(thread_args->work_queue)) {
            pthread_cond_broadcast(&thread_args->idle_cond); 
        }
        pthread_mutex_unlock(&thread_args->mut);
    }

    return NULL;
}

void thread_pool_stop(thread_pool *pool) {
    if (!pool) return;
    pthread_mutex_lock(&pool->info->mut);
    pool->info->stop_work = 1;
    pthread_cond_broadcast(&pool->info->work_cond);
    pthread_mutex_unlock(&pool->info->mut);
}

void thread_info_free(thread_info *info) {
    pthread_cond_destroy(&info->work_cond);
    pthread_mutex_destroy(&info->mut);
    queue_free(info->work_queue);
    free(info);
}

thread_info *_thread_info_init(size_t queue_cap, thread_function thread_func, void *thread_arg) {
    int err = 0;
    thread_info *info = calloc(1, sizeof(*info));

    if ((err = pthread_cond_init(&info->work_cond, NULL)) != 0) {
        free(info);
        return NULL;
    }

    if ((err = pthread_cond_init(&info->idle_cond, NULL)) != 0) {
        free(info);
        pthread_cond_destroy(&info->work_cond);
        return NULL;
    }

    if ((err = pthread_mutex_init(&info->mut, NULL)) != 0) {
        pthread_cond_destroy(&info->work_cond);
        pthread_cond_destroy(&info->idle_cond);
        free(info);
        return NULL;
    }

    if ((info->work_queue = queue_init(queue_cap)) == NULL) {
        err = 1;
        goto cleanup;
    }

    info->func = thread_func;
    info->arg = thread_arg;
    info->stop_work = 0;

cleanup:
    if (err) {
        thread_info_free(info);
        return NULL;
    }

    return info;
}

thread_pool *thread_pool_create(size_t thread_count, size_t queue_cap, thread_function thread_func, void *thread_arg) {
    int err = 0;
    thread_pool *pool = calloc(1, sizeof(*pool));
    if (!pool) return NULL;

    pool->info = _thread_info_init(queue_cap, thread_func, thread_arg);
    if ((pool->info) == NULL) {
        free(pool);
        return NULL;
    }

    pool->thread_count = thread_count;
    pool->threads = calloc(thread_count, sizeof(pthread_t));
    if (!pool->threads) {
        err = 1;
        goto cleanup;
    }

    for (size_t i = 0; i < thread_count; i++) {
        if ((err = pthread_create(&pool->threads[i], NULL, do_work, pool->info)) != 0) {
            thread_pool_stop(pool);
            for (size_t j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            goto cleanup;
        }
    }
    
cleanup:
    if (err) {
        free(pool->threads);
        thread_info_free(pool->info);
        free(pool);
        return NULL;
    }

    return pool;
}

void thread_pool_join(thread_pool *pool) {
    if (!pool) return;
    for (size_t i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}