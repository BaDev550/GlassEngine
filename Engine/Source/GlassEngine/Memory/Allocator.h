#pragma once
#include <iostream>
#include "GlassEngine/Utilities/MemoryProfiler.h"

namespace ge::mem {
	struct AllocationMetrics {
		size_t totalAllocated = 0;
		size_t totalFreed = 0;
		size_t CurrentUsage() const { return totalAllocated - totalFreed; }
	} inline s_allocationMetrics;
	struct DefaultAllocTag { static constexpr const char* Name = "GE_Allocator"; };
	struct SceneAllocTag { static constexpr const char* Name = "GE_Allocator_Scene"; };
	struct RendererAllocTag { static constexpr const char* Name = "GE_Allocator_Renderer"; };
	struct AssetAllocTag { static constexpr const char* Name = "GE_Allocator_AssetManager"; };
	struct ThreadAllocTag { static constexpr const char* Name = "GE_Allocator_ThreadManager"; };

	namespace allocFuncs {
		void* GE_Allocate(size_t size);
		void* GE_AllocateAligned(size_t size, size_t alignment);
		void* GE_ReallocateAligned(void* originalBlock, size_t size, size_t alignment);
		void GE_FreeAligned(void* block, size_t size, size_t aligment);
		void GE_Free(void* block, size_t size);
	}
#define GE_ALLOC(size, category) void* ptr = ::ge::mem::allocFuncs::GE_Allocate(size); ::ge::MemoryProfiler::Get().RecordAlloc(ptr, size, category);
#define GE_ALIGNED_ALLOC(size, alignment, category) void* ptr = ::ge::mem::allocFuncs::GE_AllocateAligned(size, alignment); ::ge::MemoryProfiler::Get().RecordAlloc(ptr, size, category);
#define GE_ALIGNED_FREE(ptr, size, alignment) ::ge::mem::allocFuncs::GE_FreeAligned(ptr, size, alignment); ::ge::MemoryProfiler::Get().RecordFree(ptr);
#define GE_FREE(ptr, size) ::ge::mem::allocFuncs::GE_Free(ptr, size); ::ge::MemoryProfiler::Get().RecordFree(ptr);

	template<typename T, typename Tag = DefaultAllocTag>
	class GE_Allocator {
	public:
		typedef T value_type;
		GE_Allocator() noexcept {}
		template<typename U>
		GE_Allocator(const GE_Allocator<U, Tag>&) noexcept {}

		T* allocate(std::size_t n) {
			GE_ALLOC(n * sizeof(T), Tag::Name);
			return static_cast<T*>(ptr);
		}
		void deallocate(T* p, std::size_t n) noexcept {
			GE_FREE(p, n * sizeof(T));
		}

		template <typename U>
		bool operator==(const GE_Allocator<U, Tag>&) const noexcept { return true; }
		template <typename U>
		bool operator!=(const GE_Allocator<U, Tag>&) const noexcept { return false; }
	};
}