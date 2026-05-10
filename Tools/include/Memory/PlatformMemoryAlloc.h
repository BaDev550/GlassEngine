#pragma once
#include <stdlib.h>

#ifdef _WIN32
#include <malloc.h>
#define GE_PLATFORM_ALLOC(size) malloc(size)
#define GE_PLATFORM_FREE(block) free(block)
#define GE_PLATFORM_ALIGNED_REALLOC(ptr, size, alignment) _aligned_realloc(ptr, size, alignment)
#define GE_PLATFORM_ALIGNED_ALLOC(size, alignment) _aligned_malloc(size, alignment)
#define GE_PLATFORM_ALIGNED_FREE(block) _aligned_free(block)

#elif defined(__linux__) || defined(__gnu_linux__)
#include <malloc.h>
#include <cstring>

#define GE_PLATFORM_ALLOC(size)                  malloc(size)
#define GE_PLATFORM_FREE(block)                  free(block)
#define GE_PLATFORM_ALIGNED_FREE(block) free(block)

static inline void* ge_linux_aligned_alloc(size_t size, size_t alignment) {
    void* ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}
static inline void* ge_linux_aligned_realloc(void* ptr, size_t size, size_t alignment) {
	void* newPtr = ge_linux_aligned_alloc(size, alignment);
	if (newPtr && ptr) {
        memcpy(newPtr, ptr, size);
        GE_PLATFORM_ALIGNED_FREE(ptr);
    }
    return newPtr;
}

#define GE_PLATFORM_ALIGNED_ALLOC(size, alignment) ge_linux_aligned_alloc(size, alignment)
#define GE_PLATFORM_ALIGNED_REALLOC(ptr, size, alignment) ge_linux_aligned_realloc(ptr, size, alignment)

#elif defined(__APPLE__) || defined(__MACH__)
#error("MAC-OS is not supported")
#else
#error("Unknown Platform")
#endif