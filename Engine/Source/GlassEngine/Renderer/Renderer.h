#pragma once
#include "GlassEngine/Renderer/RenderAPI.h"
#include "RenderPass.h"

namespace ge::renderer {
	struct RenderConfig { // TODO (0x): make this more complete this is just for testing
		bool Bloom = false;
		bool DepthOfField = false;
		bool MotionBlur = false;
	};

	class ShaderLibrary;
	class Renderer3D {
	public:
		constexpr static uint32_t MaxFramesInFlight = 2;

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
		static ge::mem::Ref<RenderPass> GetDefaultRenderPass();
		static ge::mem::Ref<RenderAPI> GetRenderAPI();
	};
}