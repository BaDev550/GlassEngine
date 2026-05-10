#include "Memory/Allocator.h"

namespace ge::mem {
    void* GEAlloc(size_t size) {
        _allocationMetrices.totalAllocated += size;
        return GE_PLATFORM_ALLOC(size);
    }

    void GEFree(void* block, size_t size) {
        _allocationMetrices.totalFreed += size;
        GE_PLATFORM_FREE(block);
    }
}