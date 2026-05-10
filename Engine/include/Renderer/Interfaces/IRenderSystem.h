#pragma once
#include "IEngineSystem.h"

#include "IBuffer.h"

namespace ge::renderer {
	enum class GraphicsAPI : uint8_t {
		Vulkan = 0,
		DX12
	};

	struct RenderStats {
		uint32_t drawCall;
	};

	class GE_API IRenderSystem : public IEngineSystem {
	public:
		virtual void DrawVertex(uint32_t vertexCount, const mem::Ref<IBuffer>& vertexBuffer) = 0;
		virtual void DrawIndex(uint32_t indexCount, const mem::Ref<IBuffer>& vertexBuffer, const mem::Ref<IBuffer>& indexBuffer) = 0;

		virtual void PushConstant(const void* ptr, uint16_t size, uint16_t offset) = 0;

		virtual void BeginCopyPass() = 0;
		virtual void EndCopyPass() = 0;
		//virtual void BeginRenderPass(const BeginRenderPassSpec&) = 0;
		//virtual void EndRenderPass() = 0;

		RenderStats GetRenderStats() { return _renderStats; }
		GraphicsAPI GetAPI() { return _api; }
		void SetAPI(GraphicsAPI api) { _api = api; }
		
		virtual void BeginDebugLabel(std::string_view label) = 0;
		virtual void EndDebugLabel() = 0;
	private:
		RenderStats _renderStats;
		GraphicsAPI _api;
	};
}