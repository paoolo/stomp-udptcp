#include "map.h"

struct map* map_new() {
    struct map *map = NULL;
    map = NEW(struct map);
    map->mutex = NEW(pthread_mutex_t);
    pthread_mutex_init(map->mutex, NULL);
    return map;
}

void map_delete(struct map* map) {
    struct map_node *next = NULL, *node = NULL;
    pthread_mutex_t *mutex = NULL;

    if (map == NULL) {
        return;
    }

    mutex = map->queue;
    pthread_mutex_lock(mutex);

    next = map->head;

    map->mutex = NULL;
    map->head = NULL;
    map->tail = NULL;
    DELETE(map);

    while (next != NULL) {
        node = next;
        next = next->next;

        node->key = NULL;
        node->value = NULL;
        node->next = NULL;
        node->prev = NULL;
        DELETE(node);
    }

    pthread_mutex_unlock(mutex);
    pthread_mutex_destroy(mutex);
    DELETE(mutex);
}

void map_put(void *key, void *value, struct map *map) {
    struct map_node *node = NULL;

    if (map == NULL) {
        return;
    }

    node = NEW(struct map_node);
    node->key = key;
    node->value = value;
    
    pthread_mutex_lock(map->mutex);
    if (map->tail != NULL) {
        map->tail->next = node;
        node->prev = map->tail;
    } else {
        map->head = node;
    }
    map->tail = node;
    pthread_mutex_unlock(map->mutex);
}

void* map_get(void *key, struct map *map) {
    struct map_node *node = NULL;
    void *value = NULL;

    if (map == NULL) {
        return NULL;
    }

    pthread_mutex_lock(map->mutex);
    node = map->head;
    while (node != NULL && value == NULL) {
        if (map_compare(node->key, key) == 0) {
            value = node->value;
        } else {
            node = node->next;
        }
    }
    pthread_mutex_unlock(map->mutex);

    return value;
}

void* map_remove(void *key, struct map *map) {
    struct map_node *node = NULL;
    void *value = NULL;

    if (map == NULL) {
        return NULL;
    }

    pthread_mutex_lock(map->mutex);
    node = map->head;
    while (node != NULL && value == NULL) {
        if (map_compare(node->key, key) == 0) {
            value = node->value;

            if (node->prev != NULL) {
                node->prev->next = node->next;
            }
            if (node->next != NULL) {
                node->next->prev = node->prev;
            }

            node->next = NULL;
            node->prev = NULL;
            node->value = NULL;
            node->key = NULL;

            DELETE(node);
        } else {
            node = node->next;
        }
    }
    pthread_mutex_unlock(map->mutex);

    return value;
}
