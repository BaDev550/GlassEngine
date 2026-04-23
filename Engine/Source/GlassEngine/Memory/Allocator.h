#pragma once
#include <iostream>

namespace ge::mem {
	struct AllocationMetrics {
		size_t totalAllocated = 0;
		size_t totalFreed = 0;
		size_t CurrentUsage() const { return totalAllocated - totalFreed; }
	} inline s_allocationMetrics;

	namespace allocFuncs {
		void* GE_Allocate(size_t size);
		void* GE_AllocateAligned(size_t size, size_t alignment);
		void* GE_ReallocateAligned(void* originalBlock, size_t size, size_t alignment);
		void GE_FreeAligned(void* block, size_t size, size_t aligment);
		void GE_Free(void* block, size_t size);
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

		template <typename U>
		bool operator==(const GE_Allocator<U>&) const noexcept { return true; }
		template <typename U>
		bool operator!=(const GE_Allocator<U>&) const noexcept { return false; }
	};
}