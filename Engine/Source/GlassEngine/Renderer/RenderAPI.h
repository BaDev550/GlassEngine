#pragma once
#include <iostream>
#include "GlassEngine/Core/Memory.h"

#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/CommandBuffer.h"
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
		virtual void BeginDefaultPass() = 0;
		virtual void EndDefaultPass() = 0;
		virtual void DrawVertex(CommandBuffer* cmd) = 0;
		virtual void DrawIndexed(CommandBuffer* cmd) = 0;

		virtual void LoadDataToTexture2D(CommandBuffer& cmd, Texture2D& texture, void* data, uint64_t dataSize) = 0;

		static inline RenderStats GetRenderStats() { return _renderStats; }
		static inline GraphicsAPI GetAPI() { return _graphicsAPI; }
		static void SetAPI(GraphicsAPI api) { _graphicsAPI = api; }
		static mem::Ref<RenderAPI> Create();
	protected:
		static RenderStats _renderStats;
		static GraphicsAPI _graphicsAPI;
	};
}
