/**
 * 
 * queue.h - Simple thread safe message queue for passing around
 *           messages between threads. Based on pthread conditionals
 *           for optimal efficiency.
 * 
 * 
 */

#ifndef CUSTOM_QUEUE_H
#define CUSTOM_QUEUE_H

#include <stdlib.h>
#include <pthread.h>

typedef struct {
    size_t              capacity;
    size_t              size;
    size_t              front;
    size_t              back;   
    void**              items;
} message_queue;

message_queue *queue_init(size_t capacity);
int queue_full(message_queue *q);
int queue_empty(message_queue *q);
int queue_enqueue(void *element, message_queue *q);
void *queue_dequeue(message_queue *q);
void print_queue(message_queue *q);
void queue_free(message_queue *q);

#endif