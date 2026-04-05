#pragma once
#include "GlassEngine/Utilities/Logger.h"
#include <iostream>

namespace ge::mem {
	struct AllocationMetrics {
		size_t totalAllocated = 0;
		size_t totalFreed = 0;
		size_t CurrentUsage() const { return totalAllocated - totalFreed; }
	} static inline s_allocationMetrics;

#define GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE 0
	namespace allocFuncs {
		static void* GE_Allocate(size_t size) {
			s_allocationMetrics.totalAllocated += size;
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
			std::cout << "[MEMORY] Allocated: " << size << std::endl;
#endif
			return malloc(size);
		}
		static void* GE_AllocateAligned(size_t size, size_t alignment) {
			s_allocationMetrics.totalAllocated += size;
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
			std::cout << "[MEMORY] Allocated with aligment: " << size << " " << alignment << std::endl;
#endif
			return _aligned_malloc(size, alignment);
		}
		static void* GE_ReallocateAligned(void* orginalBlock, size_t size, size_t alignment) {
			s_allocationMetrics.totalAllocated += size;
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
			std::cout << "[MEMORY] Reallocated: " << orginalBlock << " " << size << std::endl;
#endif
			return _aligned_realloc(orginalBlock, size, alignment);
		}
		static void GE_FreeAligned(void* block, size_t size, size_t aligment) {
			if (block == nullptr) {
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
				std::cout << "[MEMORY] \"Free\" Tried to free up uninitialized or freed memory" << std::endl;;
#endif
				return;
			}
			s_allocationMetrics.totalFreed += size;
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
			std::cout << "[MEMORY] Freed: " << size << std::endl;;
#endif
			return _aligned_free(block);
		}
		static void GE_Free(void* block, size_t size) {
			if (block == nullptr) {
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
				std::cout << "[MEMORY] \"Free\" Tried to free up uninitialized or freed memory" << std::endl;;
#endif
				return;
			}
			s_allocationMetrics.totalFreed += size;
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
			std::cout << "[MEMORY] Freed: " << size << std::endl;
#endif
			free(block);
		}
	}

	template<typename T>
	class GE_Allocator {
	public:
		typedef T value_type;
		GE_Allocator() noexcept {}
		template<typename U>
		GE_Allocator(const GE_Allocator<U>&) noexcept {}

		T* allocate(std::size_t n) {
			return static_cast<T*>(allocFuncs::GE_Allocate(n * sizeof(T)));
		}
		void deallocate(T* p, std::size_t n) noexcept {
			allocFuncs::GE_Free(p, n * sizeof(T));
		}
	};
}