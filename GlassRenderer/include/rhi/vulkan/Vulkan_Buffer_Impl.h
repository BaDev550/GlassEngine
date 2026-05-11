#pragma once

#include "IBuffer_Impl.h"

namespace ge::graphics {
	class Vulkan_Buffer_Impl final : public IBuffer_Impl {
	public:
		Vulkan_Buffer_Impl(const BufferSpec& specs);
		~Vulkan_Buffer_Impl();

	};
}