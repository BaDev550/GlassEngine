#pragma once
#include "GlassEngine/Renderer/RenderAPI.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_RenderAPI : public RenderAPI {
	public:
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void BeginDefaultPass() override;
		virtual void EndDefaultPass() override;
		virtual void DrawVertex() override;
		virtual void DrawIndexed() override;
		VkCommandBuffer GetCurrentCommandBuffer() { return VK_NULL_HANDLE; }

		virtual void LoadDataToTexture2D(Texture2D& texture, void* data, uint64_t dataSize) override {}
	};
}