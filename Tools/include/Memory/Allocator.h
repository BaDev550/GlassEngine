#pragma once
#include "PlatformMemoryAlloc.h"
#include <iostream>
#include <memory>

namespace ge::mem {
	struct AllocationMetrices {
		size_t totalAllocated = 0;
		size_t totalFreed = 0;
		size_t GetCurrentUsage() const { return (totalAllocated - totalFreed); }
	};
	inline AllocationMetrices _allocationMetrices;

	[[nodiscard]] void* GEAlloc(size_t n);
	void GEFree(void* block, size_t n);

	template<typename T>
	class GE_Allocator {
	public:
		typedef T value_type;

		GE_Allocator() noexcept {}
		template<typename U>
		GE_Allocator(const GE_Allocator<U>&) noexcept {}

		T* allocate(size_t n) {
			void* ptr = GEAlloc((sizeof(T) * n));
			return static_cast<T*>(ptr);
		}

		void deallocate(void* block, size_t n) {
			GEFree(block, (sizeof(T) * n));
		}

		bool operator==(const GE_Allocator&) const noexcept { return true; }
    	bool operator!=(const GE_Allocator&) const noexcept { return false; }
	};
}