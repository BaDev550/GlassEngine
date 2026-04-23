#include "gepch.h"
#include "Allocator.h"
#include "MemoryMacros.h"
#include "GlassEngine/Utilities/Profiler.h"

namespace ge::mem {
	namespace allocFuncs {
		void* GE_Allocate(size_t size) {
#if GE_MEMORY_ALLOCATOR_DEBUG_WRITE_INTO_PROFILER
			GE_PROFILE_SCOPE(("Mem::Alocation size: " + std::to_string(size)));
#endif
			s_allocationMetrics.totalAllocated += size;
			return GE_ALLOC(size);
		}

		void* GE_AllocateAligned(size_t size, size_t alignment) {
#if GE_MEMORY_ALLOCATOR_DEBUG_WRITE_INTO_PROFILER
			GE_PROFILE_SCOPE(("Mem::Aligned Alocation size: " + std::to_string(size) + " alignment: " + std::to_string(alignment)));
#endif
			s_allocationMetrics.totalAllocated += size;
			return GE_ALIGNED_ALLOC(size, alignment);
		}

		void* GE_ReallocateAligned(void* originalBlock, size_t size, size_t alignment) {
#if GE_MEMORY_ALLOCATOR_DEBUG_WRITE_INTO_PROFILER
			GE_PROFILE_SCOPE(("Mem::Reallocation Aligned size: " + std::to_string(size) + " alignment: " + std::to_string(alignment)));
#endif
			s_allocationMetrics.totalAllocated += size;
			return GE_ALIGNED_REALLOC(originalBlock, size, alignment);
		}

		void GE_FreeAligned(void* block, size_t size, size_t aligment) {
			if (block == nullptr) {
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
				std::cout << "[MEMORY] \"FreeAligned\" Tried to free up uninitialized or freed memory" << std::endl;
#endif
				return;
			}
			s_allocationMetrics.totalFreed += size;
			GE_ALIGNED_FREE(block);
		}

		void GE_Free(void* block, size_t size) {
			if (block == nullptr) {
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
				std::cout << "[MEMORY] \"FreeAligned\" Tried to free up uninitialized or freed memory" << std::endl;
#endif
				return;
			}
			s_allocationMetrics.totalFreed += size;
			GE_FREE(block);
		}
	}
}