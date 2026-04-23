#pragma	once

#define GE_ALLOC(size) malloc(size)
#ifdef _WIN32
	#define GE_ALIGNED_ALLOC(size, alignment) _aligned_malloc(size, alignment)
	#define GE_ALIGNED_FREE(ptr) _aligned_free(ptr)
	#define GE_ALIGNED_REALLOC(ptr, size, alignment) _aligned_realloc(ptr, size, alignment)
	#define GE_FREE(ptr) free(ptr)
#else
	#define GE_ALIGNED_ALLOC(size, alignment) \
	({ \
		void* ptr = nullptr; \
		if (posix_memalign(&ptr, alignment, size) != 0) ptr = nullptr; \
		ptr; \
	})
	#define GE_ALIGNED_REALLOC(ptr, size, alignment) \
	({ \
		void* newPtr = GE_ALIGNED_ALLOC(size, alignment); \
		if (newPtr && ptr) { \
			memcpy(newPtr, ptr, size); \
			GE_ALIGNED_FREE(ptr); \
		} \
		newPtr; \
	})
	#define GE_ALIGNED_FREE(ptr) free(ptr)
	#define GE_FREE(ptr) free(ptr)
#endif