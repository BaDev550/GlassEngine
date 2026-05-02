#pragma once
#include "GlassEngine/Renderer/RenderAPI.h"
#include "GlassEngine/Thread/RenderCommandQueue.h"
#include "RenderPass.h"

namespace ge::renderer {
	struct RenderConfig { // TODO (0x): make this more complete this is just for testing
		bool vsync = false;
		bool wireframe = false;
	};

	class ShaderLibrary;
	class Renderer3D {
	public:
		constexpr static uint32_t MaxFramesInFlight = 2;

		static void Init();
		static void Destroy();
		static void Submit(std::function<void()> func);

		static void EndFrame();
		
		static void BeginDefaultPass();
		static void EndDefaultPass();
		static void WaitAndRender();
		static void DrawVertex(ge::mem::Ref<Pipeline>& pipeline, uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
		static void DrawIndexed(ge::mem::Ref<Pipeline>& pipeline, uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex = 0, uint32_t firstInstance = 0, int32_t vertexOffset = 0);
		static void DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex = 0, ge::mem::Ref<MaterialTable> materialTable = nullptr, const glm::mat4& transform = glm::mat4(1.0f));
		static ImTextureID GetImGuiTexture(ge::mem::Ref<Image>& image);

		static uint32_t GetFrameIndex();
		static RenderConfig GetRenderConfig();
		static RenderStats GetRenderStats();
		static ShaderLibrary& GetShaderLibrary();
		static ge::mem::Ref<Texture2D>& GetWhiteTexture();
		static ge::mem::Ref<RenderAPI> GetRenderAPI();
	};
}