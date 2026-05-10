#pragma once

#include "Ref.h"
#include "Allocator.h"

namespace ge::mem {
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename...>
	Scope<T> CreateScope(Args&&... args) {
		return std::make_uniqe<T>(std::forward<Args>(args)...);
	}
}