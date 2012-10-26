#include "queue.h"
#include "tools.h"

struct queue* queue_new() {
    struct queue *queue = NULL;
    queue = NEW(struct queue);
    queue->mutex = NEW(pthread_mutex_t);
    queue->cond = NEW(pthread_cond_t);
    pthread_mutex_init(queue->mutex, NULL);
    pthread_cond_init(queue->cond, NULL);
    return queue;
}

void queue_delete(struct queue *queue) {
    struct queue_node *next = NULL, *node = NULL;
    pthread_mutex_t *mutex = NULL;

    if (queue == NULL) {
        return;
    }

    mutex = queue->mutex;
    pthread_mutex_lock(mutex);

    next = queue->head;

    pthread_cond_destroy(queue->cond);
    queue->cond = NULL;
    queue->mutex = NULL;
    queue->head = NULL;
    queue->tail = NULL;
    DELETE(queue);

    while (next != NULL) {
        node = next;
        next = node->next;

        node->handle = NULL;
        node->next = NULL;
        DELETE(node);
    }

    pthread_mutex_unlock(mutex);
    pthread_mutex_destroy(mutex);
    DELETE(mutex);
}

void queue_add(void *handle, struct queue *queue) {
    struct queue_node *node = NULL;

    if (queue == NULL) {
        return;
    }

    node = NEW(struct queue_node);
    node->handle = handle;

    pthread_mutex_lock(queue->mutex);
    if (queue->tail != NULL) {
        queue->tail->next = node;
    } else {
        queue->head = node;
        pthread_cond_broadcast(queue->cond);
    }
    queue->tail = node;
    pthread_mutex_unlock(queue->mutex);
}

void* queue_get(struct queue *queue) {
    struct queue_node *node = NULL;
    void *handle = NULL;

    if (queue == NULL) {
        return NULL;
    }

    pthread_mutex_lock(queue->mutex);

    while (queue->head == NULL) {
        pthread_cond_wait(queue->cond, queue->mutex);
    }

    node = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    handle = node->handle;
    node->handle = NULL;
    node->next = NULL;
    DELETE(node);

    pthread_mutex_unlock(queue->mutex);
    
    return handle;
}
