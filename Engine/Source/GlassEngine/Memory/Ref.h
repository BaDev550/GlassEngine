#pragma once
#include "Allocator.h"

namespace ge::mem {
	class RefCounted { // Base class for all mem::Ref objects
	public:
		RefCounted() : _refCount(0) {}
		virtual ~RefCounted() = default;

		void IncRef() const { _refCount++; }
		void DecRef() const {
			_refCount--;
			if (_refCount == 0)
				delete this;
				//allocFuncs::GE_Free((void*)this, sizeof(*this));
		}
		uint32_t GetRefCount() const { return _refCount; }
	private:
		mutable std::atomic<uint32_t> _refCount = 0;
	};

	template<typename T>
	class Ref {
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
}