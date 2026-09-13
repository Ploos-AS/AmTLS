#ifndef AMTLS_ALLOCATOR_H
#define AMTLS_ALLOCATOR_H

#include <stddef.h>

typedef void *(*AmTLS_AllocFn)(void *user, size_t size);
typedef void (*AmTLS_FreeFn)(void *user, void *ptr, size_t size);

typedef struct AmTLS_AllocatorStats {
    size_t current_bytes;
    size_t peak_bytes;
    size_t total_bytes;
    unsigned long allocations;
    unsigned long frees;
} AmTLS_AllocatorStats;

typedef struct AmTLS_Allocator {
    void *user;
    AmTLS_AllocFn alloc;
    AmTLS_FreeFn free;
    AmTLS_AllocatorStats stats;
} AmTLS_Allocator;

void *amtls_allocator_alloc(AmTLS_Allocator *allocator, size_t size);
void amtls_allocator_free(AmTLS_Allocator *allocator, void *ptr, size_t size);

#endif
