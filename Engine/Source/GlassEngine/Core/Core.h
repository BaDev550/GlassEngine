#pragma once
#include "GlassEngine/Utilities/Logger.h"
#include "GlassEngine/Core/Memory.h"
#include <vector>
#include <string>

#define GE_ASSERT(x, ...) \
	if (!(x)) {\
		GE_CORE_CRITICAL(__VA_ARGS__); \
		__debugbreak(); }

template<typename T>
using GEVector = std::vector<T, ge::mem::GE_Allocator<T>>;
using GEString = std::basic_string<char, std::char_traits<char>, ge::mem::GE_Allocator<char>>; // UTF8
// TODO (badev): make a GEWString class using wstring utf16 for localization