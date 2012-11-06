#ifndef QUEUE_H
#define QUEUE_H

struct queue_node {
    struct queue_node *next;
    void *handle;
};

struct queue {
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    struct queue_node *head, *tail;
};

struct queue* queue_new();

void queue_delete(struct queue *queue);

void queue_add(void *handle, struct queue *queue);

void* queue_get(struct queue *queue);

#endif
