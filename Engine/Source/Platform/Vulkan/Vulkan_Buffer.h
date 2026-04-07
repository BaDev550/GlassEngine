#pragma once
#include "GlassEngine/Renderer/Buffer.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_Buffer final : public Buffer {
	public:
		Vulkan_Buffer(const BufferSpec& desc);
		virtual ~Vulkan_Buffer();
	private:
		VkBuffer _buffer;
		VmaAllocation _allocation;
	};
}