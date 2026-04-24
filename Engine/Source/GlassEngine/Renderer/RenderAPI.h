#pragma once
#include <cstdint>
#include <iostream>
#include "GlassEngine/Memory/Memory.h"

#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Texture.h"

namespace ge::renderer {
	struct RenderStats {
		uint32_t drawCalls;
	};

	enum class GraphicsAPI {
		Vulkan,
		OpenGL,
		DirectX11,
		DirectX12
	};

	class RenderAPI : public RenderObject {
	public:
		RenderAPI() = default;
		virtual ~RenderAPI() = default;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t firstInstance = 0, int32_t vertexOffset = 0) = 0;

		virtual void BeginCopyPass() = 0;
		virtual void EndCopyPass() = 0;
		// virtual void BeginRenderPass() = 0;
		// virtual void EndRenderPass() = 0;

		virtual void LoadDataToTexture2D(Texture2D& texture, void* data, uint64_t dataSize) = 0;

		static inline RenderStats GetRenderStats() { return _renderStats; }
		static inline GraphicsAPI GetAPI() { return _graphicsAPI; }
		static void SetAPI(GraphicsAPI api) { _graphicsAPI = api; }
		static ge::mem::Ref<RenderAPI> Create();

		virtual void SetDebugName(GEString name) const noexcept final {}
	protected:
		static RenderStats _renderStats;
		static GraphicsAPI _graphicsAPI;
	};
}
