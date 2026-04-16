#pragma once
#include "GlassEngine/Renderer/RenderAPI.h"
#include "ShaderLibrary.h"

namespace ge::renderer {
	class Renderer3D {
	public:
		static void Init();
		static void Destroy();
		
		static void BeginFrame();
		static void EndFrame();
		static void BeginDefaultPass();
		static void EndDefaultPass();
		static void DrawVertex();
		static void DrawIndexed();

		static uint32_t GetFrameIndex();
		static RenderStats GetRenderStats();
		static ShaderLibrary& GetShaderLibrary(); 
		static ge::mem::Ref<RenderAPI> GetRenderAPI();
	};
}