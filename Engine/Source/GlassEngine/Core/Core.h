#pragma once
#include "String.h"
#include <vector>
#include <map>
#include <unordered_map>
#include "GlassEngine/Memory/Allocator.h"

#define GE_CONSOLE_ASSETMANAGER_CATAGORY "asset"
#define GE_CONSOLE_ENGINE_CATAGORY "engine"
#define GE_MEMORY_ALLOCATOR_DEBUG_ALLOCATION_FREE 0
#define GE_MEMORY_ALLOCATOR_DEBUG_WRITE_INTO_PROFILER 0

#ifdef _WIN32
    #define GE_DEBUGBREAK() __debugbreak()
#else
    #include <csignal>
    #define GE_DEBUGBREAK() raise(SIGTRAP)
#endif
#define GE_ASSERT(condition, msg) do { if (!(condition)) { GE_DEBUGBREAK(); std::cerr << msg << std::endl; } } while(0)
	
#define BIT(x) 1 << x
#if defined(_MSC_VER)
	#define GE_ARGS(...) ,__VA_ARGS__
#else
	#define GE_ARGS(...) __VA_OPT__(, ) __VA_ARGS__
#endif

template<typename T, typename Tag = ge::mem::DefaultAllocTag>
using GEVector = std::vector<T, ge::mem::GE_Allocator<T, Tag>>;

template<typename T, typename U, typename Tag = ge::mem::DefaultAllocTag>
using GEUnorderedMap = std::unordered_map<T, U, std::hash<T>, std::equal_to<T>, ge::mem::GE_Allocator<std::pair<const T, U>, Tag>>;

template<typename T, typename U, typename Tag = ge::mem::DefaultAllocTag>
using GEMap = std::map<T, U, std::less<T>, ge::mem::GE_Allocator<std::pair<const T, U>, Tag>>;

template<typename T>
static T* CastChecked(void* ptr) {
	T* result = static_cast<T*>(ptr);
	GE_ASSERT(result, "Cast failed");
	return result;
}