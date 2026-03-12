#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "queue.h"

#include <pthread.h>

typedef void * (*thread_function)(void *message, void *args);

typedef struct {
    void                *arg;
    thread_function     func;
    int                 stop_work;
    pthread_cond_t      work_cond;
    pthread_mutex_t     mut;
    message_queue       *work_queue;
} thread_info;

typedef struct {
    pthread_t           *threads; 
    size_t              thread_count;
    thread_info         *info;
} thread_pool;


#endif