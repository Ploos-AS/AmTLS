#include "allocator.h"

void *amtls_allocator_alloc(AmTLS_Allocator *allocator, size_t size)
{
    void *ptr;

    if (!allocator || !allocator->alloc || size == 0) {
        return 0;
    }

    ptr = allocator->alloc(allocator->user, size);
    if (!ptr) {
        return 0;
    }

    allocator->stats.current_bytes += size;
    allocator->stats.total_bytes += size;
    allocator->stats.allocations++;
    if (allocator->stats.current_bytes > allocator->stats.peak_bytes) {
        allocator->stats.peak_bytes = allocator->stats.current_bytes;
    }

    return ptr;
}

void amtls_allocator_free(AmTLS_Allocator *allocator, void *ptr, size_t size)
{
    if (!allocator || !allocator->free || !ptr) {
        return;
    }

    allocator->free(allocator->user, ptr, size);
    if (size <= allocator->stats.current_bytes) {
        allocator->stats.current_bytes -= size;
    } else {
        allocator->stats.current_bytes = 0;
    }
    allocator->stats.frees++;
}
