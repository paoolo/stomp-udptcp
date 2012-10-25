#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

void* tools_new(int size) {
    void *ptr = NULL;

    ptr = malloc(size);
    CLEAR(ptr, size);

    return ptr;
}

void tools_delete(void *ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
}

void* tools_memcpy(void *destination, const void *source, size_t num) {
    if (source == NULL) {
        return destination;
    }
    return memcpy(destination, source, num);
}

int tools_str_length(char *str) {
    if (str != NULL) {
        return strlen(str);
    }
    return 0;
}

char* tools_str_resize(char *str, int new_length) {
    char *str_result = NULL;
    int str_length = 0;

    str_length = tools_str_length(str);
    str_result = NEW_ARRAY(char, new_length);

    if (new_length > str_length) {
        tools_memcpy(str_result, str, str_length);
    } else {
        tools_memcpy(str_result, str, new_length);
    }

    return str_result;
}

char* tools_str_expand(char *str, int expand) {
    int str_length = tools_str_length(str);
    return tools_str_resize(str, str_length + expand);
}

char* tools_str_truncate(char *str) {
    int str_length = tools_str_length(str);
    return tools_str_resize(str, str_length);
}
