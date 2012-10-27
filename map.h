#ifndef MAP_H
#define MAP_H

struct map_node {
    struct map_node *next, *prev;
    void *key, *value;
};

struct map {
    struct map_node *head, *tail;
    pthread_mutex_t *mutex;
};

struct map* map_new();

void map_delete(struct map* map);

void map_put(void *key, void *value, struct map *map);

void* map_get(void *key, struct map *map, int(*map_compare)(void*, void*));

void* map_remove(void *key, struct map *map, int(*map_compare)(void*, void*));

#endif
