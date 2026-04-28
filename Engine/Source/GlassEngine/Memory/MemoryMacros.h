#pragma	once

#define GE_PLATFORM_ALLOC(size) malloc(size)
#ifdef _WIN32
	#define GE_PLATFORM_ALIGNED_ALLOC(size, alignment) _aligned_malloc(size, alignment)
	#define GE_PLATFORM_ALIGNED_FREE(ptr) _aligned_free(ptr)
	#define GE_PLATFORM_ALIGNED_REALLOC(ptr, size, alignment) _aligned_realloc(ptr, size, alignment)
	#define GE_PLATFORM_FREE(ptr) free(ptr)
#else
	#define GE_PLATFORM_ALIGNED_FREE(ptr) free(ptr)
	#define GE_PLATFORM_FREE(ptr) free(ptr)
	#define GE_PLATFORM_ALIGNED_ALLOC(size, alignment) \
	({ \
		void* ptr = nullptr; \
		if (alignment == 1)			\
			ptr = GE_PLATFORM_ALLOC(size);			\
		else if (posix_memalign(&ptr, alignment, size) != 0) ptr = nullptr; \
		ptr; \
	})
	#define GE_PLATFORM_ALIGNED_REALLOC(ptr, size, alignment) \
	({ \
		void* newPtr = GE_PLATFORM_ALIGNED_ALLOC(size, alignment); \
		if (newPtr && ptr) { \
			memcpy(newPtr, ptr, size); \
			GE_PLATFORM_ALIGNED_FREE(ptr); \
		} \
		newPtr; \
	})
#endif