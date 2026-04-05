#pragma once
#include "GlassEngine/Utilities/Logger.h"
#include <iostream>
#include <atomic>

#define GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE 0
namespace ge::mem {
	struct AllocationMetrics {
		size_t totalAllocated = 0;
		size_t totalFreed = 0;
		size_t CurrentUsage() const { return totalAllocated - totalFreed; }
	} static inline s_allocationMetrics;

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
				std::cout << "[MEMORY] \"Free\" Tried to free up uninitialized or freed memory" << std::endl;
				return;
			}
			s_allocationMetrics.totalFreed += size;
#if GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE
			std::cout << "[MEMORY] Freed: " << size << std::endl;
#endif
			return _aligned_free(block);
		}
		static void GE_Free(void* block, size_t size) {
			if (block == nullptr) {
				std::cout << "[MEMORY] \"Free\" Tried to free up uninitialized or freed memory" << std::endl;
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

	class RefCounted { // Base class for all mem::Ref objects
	public:
		RefCounted() : _refCount(0) {}
		virtual ~RefCounted() = default;

		void IncRef() const { _refCount++; }
		void DecRef() const { 
			_refCount--;
			if (_refCount == 0)
				delete this; // TODO (0x): change this so it uses the custom allocator but it needs a new class imp so idk
		}
		uint32_t GetRefCount() const { return _refCount; }
	private:
		mutable std::atomic<uint32_t> _refCount = 0;
	};

	template<typename T>
	class Ref { // Consider making the name GERef idk
	public:
		Ref() : _instance(nullptr) {}
		Ref(std::nullptr_t) : _instance(nullptr) {}
		Ref(T* instance) : _instance(instance) {
			IncRef();
		}
		Ref(const Ref<T>& other) : _instance(other._instance) {
			IncRef();
		}
		Ref(Ref<T>&& other) noexcept : _instance(other._instance) {
			other._instance = nullptr;
		}
		template<typename T2>
		Ref(const Ref<T2>& other) {
			_instance = (T*)other._instance;
			IncRef();
		}
		template<typename T2>
		Ref(Ref<T2>&& other) {
			_instance = (T*)other._instance;
			other._instance = nullptr;
		}
		~Ref() { DecRef(); }
		Ref& operator=(const Ref<T>& other) {
			if (this == &other) return *this;
			other.IncRef();
			DecRef();
			_instance = other._instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(const Ref<T2>& other) {
			other.IncRef();
			DecRef();
			_instance = other._instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(Ref<T2>&& other) {
			DecRef();
			_instance = other._instance;
			other._instance = nullptr;
			return *this;
		}

		T* operator->() { return _instance; }
		const T* operator->() const { return _instance; }
		T& operator*() { return *_instance; }
		const T& operator*() const { return *_instance; }

		operator bool() const { return _instance != nullptr; }

		T* Get() { return _instance; }
		bool operator==(const Ref<T>& other) const { return _instance == other._instance; }
		bool operator!=(const Ref<T>& other) const { return _instance != other._instance; }

		template<typename T2>
		Ref<T2> Cast() const {
			return Ref<T2>(*this);
		}

		template<typename... Args>
		static Ref<T> Create(Args&&... args) {
			GE_Allocator<T> refAllocator;
			T* memory = refAllocator.allocate(1);
			T* instance = new(memory) T(std::forward<Args>(args)...);
			return instance;
		}
		void IncRef() const {
			static_assert(std::is_base_of<RefCounted, T>::value, "instance is not ref counted");
			if (_instance) _instance->IncRef();
		}
		void DecRef() const {
			static_assert(std::is_base_of<RefCounted, T>::value, "instance is not ref counted");
			if (_instance) {
				_instance->DecRef();
			}
		}
		mutable T* _instance;
	};

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	Scope<T> CreateScope(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}
