#include "queue.h"
#include "logger.h"

#include <stdio.h>

message_queue *queue_init(size_t capacity) {
    if (capacity == 0) {
        return NULL;
    }

    message_queue *queue = calloc(1, sizeof(*queue));
    
    if (!queue) {
        // Failed to allocate space for queue.
        return NULL;
    }
    
    queue->items = calloc(capacity, sizeof(void*));
    if (!queue->items) {
        free(queue);
        return NULL;
    }

    queue->capacity = capacity;
    queue->front = 0;
    queue->back = 0;
    queue->size = 0;

    return queue;
}
int queue_full(message_queue *q) {
    return q->size == q->capacity;
}
int queue_empty(message_queue *q) {
    return q->size == 0;
}

int queue_enqueue(void *element, message_queue *q) {
    if (!q || !element) return 1;
    if (queue_full(q)) {
        return 1;
    }
    q->items[q->back] = element;
    // Increment the rear pointer
    q->back++;
    q->back %= q->capacity;
    q->size++;

    return 0;
}

void *queue_dequeue(message_queue *q) {
    if (!q) {
        return NULL;
    }
    if (queue_empty(q)) {
        return NULL;
    }
    void *element = q->items[q->front];
    q->items[q->front] = NULL;
    q->front++;
    q->front %= q->capacity;
    q->size--;
    return element;
}

void queue_free(message_queue *q) {
    free(q->items);
    free(q);
}

void print_queue(message_queue *q) {
    if (!q) {
        log_debug("NULL pointer exception");
        return;
    }
    if (queue_empty(q)) log_debug("Queue is empty!");

    log_debug("Queue:\n");
    for (size_t i = 0; i < q->capacity; i++) {
        log_debug("\t %lu. ", i + 1);
        if (!q->items[i]) {
            log_debug("NULL ");
        } else {
            log_debug("%d ", *(int *)q->items[i]);
        }

        if (i == q->back) {
            log_debug(" <- BACK");
        }

        if (i == q->front) {
            log_debug(" <- FRONT");
        }
        log_debug("\n");
    }
}