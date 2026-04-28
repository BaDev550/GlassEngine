#pragma once
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#include <execinfo.h>
#include <cstdlib>
#endif

namespace ge {
	struct AllocRecord {
		size_t size;
		const char* category;
		void* stackTrace[6];
		int frameCount;
	};

	class MemoryProfiler {
	public:
		static MemoryProfiler& Get() {
			static MemoryProfiler instance;
			return instance;
		}

		static void Destroy() {
			Get()._activeAllocations.clear();
			Get()._categoryUsage.clear();
		}

		void RecordAlloc(void* ptr, size_t size, const char* category) {
			AllocRecord record;
			record.size = size;
			record.category = category;
#ifdef _WIN32
			record.frameCount = CaptureStackBackTrace(2, 6, record.stackTrace, NULL);
#else
			record.frameCount = backtrace(record.stackTrace, 6);
#endif
			_activeAllocations[ptr] = record;
			_categoryUsage[category] += size;
			_totalAllocated += size;
		}

		void RecordFree(void* ptr) {
			auto it = _activeAllocations.find(ptr);
			if (it != _activeAllocations.end()) {
				_categoryUsage[it->second.category] -= it->second.size;
				_totalAllocated -= it->second.size;
				_activeAllocations.erase(it);
			}
		}

		size_t GetTotalAllocated() { return _totalAllocated; }
		const std::unordered_map<void*, AllocRecord>& GetActiveAllocations() const { return _activeAllocations; }
		const std::unordered_map<std::string, size_t>& GetCategoryUsage() const { return _categoryUsage; }
	private:

		std::unordered_map<void*, AllocRecord> _activeAllocations;
		std::unordered_map<std::string, size_t> _categoryUsage;
		size_t _totalAllocated = 0;
	};
}