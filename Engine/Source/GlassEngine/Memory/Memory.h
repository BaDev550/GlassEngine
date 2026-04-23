#pragma once
#include <memory>
#include "Ref.h"
#include "Allocator.h"

namespace ge::mem {
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	Scope<T> CreateScope(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}