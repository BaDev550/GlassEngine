#pragma once
#include "GlassEngine/Utilities/Logger.h"

#define GE_ASSERT(x, ...) \
	if (!(x)) {\
		GE_CORE_CRITICAL(__VA_ARGS__); \
		__debugbreak(); }