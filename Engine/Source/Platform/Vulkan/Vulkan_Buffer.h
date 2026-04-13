#pragma once
#include "GlassEngine/Renderer/Buffer.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_Buffer final : public Buffer {
	public:
		Vulkan_Buffer(const BufferSpec& desc);
		~Vulkan_Buffer();

		[[nodiscard]] auto GetVkBuffer() const noexcept { return _buffer; }
		[[nodiscard]] auto *GetAllocation() const noexcept { return _allocation; }
	private:
		VkBuffer _buffer;
		VmaAllocation _allocation;
	};

	inline Vulkan_Buffer::~Vulkan_Buffer() {
		VK_ALLOCATOR.DestroyBuffer(_buffer, _allocation);
	}
}