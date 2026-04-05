#pragma once
#include "GlassEngine/Utilities/Logger.h"
#include "GlassEngine/Memory/Allocator.h"
#include "GlassEngine/Utilities/Profiller.h"
#include <vector>
#include <string>
#include <deque>
#include <map>

#define GE_ASSERT(x, ...) \
	if (!(x)) {\
		GE_CORE_CRITICAL(__VA_ARGS__); \
		__debugbreak(); }
#define BIT(x) 1 << x
#define GE_PROFILE_SCOPE(name) ge::profiler::ProfileScope profileScope##__LINE__(name)

template<typename T>
using GEVector = std::vector<T, ge::mem::GE_Allocator<T>>;
template<typename T>
using GEDequqe = std::deque<T, ge::mem::GE_Allocator<T>>;
using GEString = std::basic_string<char, std::char_traits<char>, ge::mem::GE_Allocator<char>>; // UTF8
// TODO (badev): make a GEWString class using wstring utf16 for localization

template<typename T>
static T* CastChecked(void* ptr) {
	T* result = static_cast<T*>(ptr);
	GE_ASSERT(result, "Cast failed");
	return result;
}

namespace ge::mem {
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	Scope<T> CreateScope(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}