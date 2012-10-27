#ifndef TOOLS_H
#define TOOLS_H

void tools_memset(void * ptr, int value, size_t num);

void* tools_memcpy(void *destination, const void *source, size_t num);

#define CLEAR(ptr, size) tools_memset(ptr, 0, size)

void* tools_new(int size);

void tools_delete(void *ptr);

#define NEW(T) (T*)tools_new(sizeof(T))
#define NEW_ARRAY(T, LEN) (T*)tools_new(sizeof(T) * (LEN))
#define DELETE(PTR) tools_delete(PTR)

int tools_str_length(char *str);

char* tools_str_resize(char *str, int new_length);

char* tools_str_expand(char *str, int expand);

char* tools_str_truncate(char *str);

#endif
