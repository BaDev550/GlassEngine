#pragma once
#include "GlassEngine/Core/Memory.h"
#include <vector>
#include <string>
#include <string_view>

#define GE_CONSOLE_ASSETMANAGER_CATAGORY "asset"
#define GE_CONSOLE_ENGINE_CATAGORY "engine"

#ifdef _WIN32
    #define GE_DEBUGBREAK() __debugbreak()
#else
    #include <csignal>
    #define GE_DEBUGBREAK() raise(SIGTRAP)
#endif

#define GE_ASSERT(condition, msg) \
    do { if (!(condition)) { GE_DEBUGBREAK(); } } while(0)
	
#define BIT(x) 1 << x

template<typename T>
using GEVector = std::vector<T, ge::mem::GE_Allocator<T>>;
using GEString = std::basic_string<char, std::char_traits<char>, ge::mem::GE_Allocator<char>>; // UTF8
// TODO (badev): make a GEWString class using wstring utf16 for localization

template<typename T>
static T* CastChecked(void* ptr) {
	T* result = static_cast<T*>(ptr);
	GE_ASSERT(result, "Cast failed");
	return result;
}