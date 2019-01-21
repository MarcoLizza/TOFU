#include "memory.h"

#include <memory.h>

void *Memory_alloc(size_t size)
{
    return Memory_realloc(NULL, size);
}

void Memory_free(void *ptr)
{
    Memory_realloc(ptr, 0);
}

void *Memory_realloc(void *ptr, size_t size)
{
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, size);
}

void *Memory_clone(const void *ptr, size_t size)
{
    void *clone = Memory_alloc(size);
    memcpy(clone, ptr, size);
    return clone;
}
