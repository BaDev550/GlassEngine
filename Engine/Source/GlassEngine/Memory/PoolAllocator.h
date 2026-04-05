#pragma once
#include "Allocator.h"
#include <iostream>

#include "GlassEngine/Utilities/Logger.h"

namespace ge::mem {
	struct FreeNode {
		FreeNode* next;
	};
	class PoolAllocator {
	public:
		PoolAllocator(size_t objectSize, size_t objectCount, const char* debugName = "OBJpool") : _objectSize(objectSize), _debugName(debugName) {
			_objectSize = std::max(objectSize, sizeof(FreeNode));
			_poolStart = allocFuncs::GE_Allocate(_objectSize * objectCount);
			_head = static_cast<FreeNode*>(_poolStart);

			FreeNode* current = _head;
			for (size_t i = 0; i < objectCount - 1; i++) {
				uintptr_t nextAddress = reinterpret_cast<uintptr_t>(current) + _objectSize;
				current->next = reinterpret_cast<FreeNode*>(nextAddress);
				current = current->next;
			}
			current->next = nullptr;
		}
		void* Allocate() {
			if (_head == nullptr) { 
				GE_CORE_CRITICAL("Object pool {} is full!!", _debugName);
				return nullptr; 
			}
			FreeNode* node = _head;
			_head = _head->next;
			return static_cast<void*>(node);
		}
		void Free(void* ptr) {
			FreeNode* node = static_cast<FreeNode*>(ptr);
			node->next = _head;
			_head = node;
		}
	private:
		const char* _debugName;
		size_t _objectSize;
		void* _poolStart;
		FreeNode* _head;
	};
}