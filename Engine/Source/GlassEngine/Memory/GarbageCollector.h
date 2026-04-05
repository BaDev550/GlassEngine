#pragma once

namespace ge::mem {
#ifdef GE_USE_MEMORY_GARBAGE_COLLECTOR // mybe using in scripting engine
	class PtrObject {
	public:
		PtrObject() : _marked(false) {}
		virtual ~PtrObject() = default;

		void Use() { _marked = true; }
		void Release() { _marked = false; }
		bool _marked;
	};

	class GarbageCollector {
	public:
		void Collect() {
			for (auto it = _allocatedObjects.begin(); it != _allocatedObjects.end()) {
				PtrObject* obj = static_cast<PtrObject*>(*it);
				if (!obj->_marked) {
#if GE_MEMORY_GARBAGE_COLLECTOR
					std::cout << "Collecting object at " << obj << std::endl;
#endif
					delete obj;
					it = _allocatedObjects.erase(it);
				}
				else {
					obj->_marked = false;
					it++;
				}
			}
		}

		static GarbageCollector& GetGC() {
			static GarbageCollector gc;
			return gc;
		}
		void AddToList(void* obj) {
			GarbageCollector::GetGC().GetAllocatedObjects().push_back(obj);
		}
		void RemoveFromList(void* obj) {
			_allocatedObjects.erase(std::remove(_allocatedObjects.begin(), _allocatedObjects.end(), obj), _allocatedObjects.end());
		}
		std::vector<void*>& GetAllocatedObjects() { return _allocatedObjects; }
	private:
		std::vector<void*> _allocatedObjects;
	};

	void AllocatePtrObject(void* obj) {
		GarbageCollector::GetGC().AddToList(obj);
	}
#endif
}