#pragma once
#include <map>
#include <vector>
#include <unordered_map>

#include "Memory/Allocator.h"

namespace ge {
    template<typename T>
    using GEVector = ::std::vector<T, ge::mem::GE_Allocator<T>>;

    template<typename T, typename U>
    using GEUnorderedMap = ::std::unordered_map<T, U, std::hash<T>, std::equal_to<T>, ge::mem::GE_Allocator<std::pair<const T, U>>>;

    template<typename T, typename U>
    using GEMap = ::std::map<T, U, std::less<T>, ge::mem::GE_Allocator<std::pair<const T, U>>>;

    using GEString = ::std::basic_string<char, std::char_traits<char>, ge::mem::GE_Allocator<char>>;
}

#ifdef _WIN32
#define GE_DEBUGBREAK() __debugbreak()
#else
#include <csignal>
#define GE_DEBUGBREAK() raise(SIGTRAP)
#endif
#define GE_ASSERT(condition, msg) do { if (!(condition)) { GE_DEBUGBREAK(); std::cerr << msg << std::endl; } } while(0)	
#define BIT(x) 1 << x

#ifdef GE_EXPORT
#define GE_API __declspec(dllexport)
#else
#define GE_API __declspec(dllimport)
#endif

template<typename T>
static T* CastChecked(void* ptr) {
	T* result = static_cast<T*>(ptr);
	GE_ASSERT(result, "Cast failed");
	return result;
}