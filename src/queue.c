#include "queue.h"

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
        printf("QUEUE OBJECT IS NULL\n");   
        return NULL;
    }
    if (queue_empty(q)) {
        printf("EMPTY QUEUE\n");
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
        printf("NULL pointer exception\n");
        return;
    }
    if (queue_empty(q)) printf("Queue is empty!\n");

    printf("Queue:\n");
    for (size_t i = 0; i < q->capacity; i++) {
        printf("\t %lu. ", i + 1);
        if (!q->items[i]) {
            printf("NULL ");
        } else {
            printf("%d ", *(int *)q->items[i]);
        }

        if (i == q->back) {
            printf(" <- BACK");
        }

        if (i == q->front) {
            printf(" <- FRONT");
        }
        printf("\n");
    }
}