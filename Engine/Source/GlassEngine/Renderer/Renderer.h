#pragma once
#include "GlassEngine/Renderer/CommandBuffer.h"
#include "GlassEngine/Renderer/RenderAPI.h"

namespace ge::renderer {
	class Renderer3D {
	public:
		static void Init();
		static void Destroy();
		
		static void BeginFrame();
		static void EndFrame();
		static void BeginDefaultPass();
		static void EndDefaultPass();
		static void DrawVertex(CommandBuffer* cmd);
		static void DrawIndexed(CommandBuffer* cmd);

		static RenderStats GetRenderStats();
		static mem::Ref<RenderAPI> GetRenderAPI();
	};
}