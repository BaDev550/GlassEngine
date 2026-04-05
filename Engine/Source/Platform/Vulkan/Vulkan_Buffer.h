#pragma once
#include "GlassEngine/Renderer/Buffer.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_Buffer : public Buffer {
	public:
		Vulkan_Buffer(const BufferCreateDesc& desc);
		virtual ~Vulkan_Buffer();
	private:
		VkBuffer _buffer;
		VmaAllocation _allocation;
	};
}