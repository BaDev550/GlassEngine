#pragma once
#include "GlassEngine/Renderer/RenderAPI.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_RenderAPI : public RenderAPI {
	public:
		Vulkan_RenderAPI();
		~Vulkan_RenderAPI();
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void DrawVertex() override;
		virtual void DrawIndexed() override;
		VkCommandBuffer GetCurrentCommandBuffer();

		virtual void LoadDataToTexture2D(Texture2D& texture, void* data, uint64_t dataSize) override {}
	private:
		struct FrameContext {
			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;
		} _frames[Renderer3D::MaxFramesInFlight]; // TEMP ig
		bool _frameStarted = false;
	};
}