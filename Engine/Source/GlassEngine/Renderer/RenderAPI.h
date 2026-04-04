#pragma once
#include <iostream>
#include "GlassEngine/Core/Memory.h"

#include "GlassEngine/Renderer/CommandBuffer.h"

namespace ge::renderer {
	enum class GraphicsAPI {
		Vulkan,
		OpenGL,
		DirectX11
	};

	class RenderAPI {
	public:
		RenderAPI() = default;
		virtual ~RenderAPI() = default;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void BeginDefaultPass() = 0;
		virtual void EndDefaultPass() = 0;
		virtual void DrawVertex(CommandBuffer* cmd) = 0;
		virtual void DrawIndexed(CommandBuffer* cmd) = 0;

		static inline GraphicsAPI GetAPI() { return _graphicsAPI; }
	private:
		static inline GraphicsAPI _graphicsAPI = GraphicsAPI::Vulkan;
	};
}