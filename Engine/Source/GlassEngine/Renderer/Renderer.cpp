#include "Renderer.h"
#include "ShaderLibrary.h"

namespace ge::renderer {
	struct RenderData {
		mem::Scope<ShaderLibrary> _shaderLibrary = nullptr;
	} static s_data;
	static mem::Ref<RenderAPI> g_renderAPI = nullptr;
	static mem::Ref<RenderPass> g_defaultRenderPass = nullptr;
	static uint32_t g_frameIndex = 0;

	void Renderer3D::Init() {
		GE_PROFILE_SCOPE("Renderer3D::Init");

		g_renderAPI = RenderAPI::Create();
		s_data._shaderLibrary = mem::CreateScope<ShaderLibrary>();

		GE_ADD_CONSOLE_COMMAND("r", "wireframe", [](const GEVector<GEString>& args) {
			bool enable = (args[0] == "1" || args[0] == "true");
			GE_GRAPHICS_INFO("Wireframe: {}", enable);
		}, "wireframe <enabled>");

		GE_ADD_CONSOLE_COMMAND("r", "vsync", [](const GEVector<GEString>& args) {
			bool enable = (args[0] == "1" || args[0] == "true");
			GE_GRAPHICS_INFO("Vsync: {}", enable);
		}, "vsync <enabled>");

		GE_ADD_CONSOLE_COMMAND("r", "reload_shaders", [](const GEVector<GEString>& args) { GetShaderLibrary().ReloadAll(); });
		GE_ADD_CONSOLE_COMMAND("r", "reload_shader", [](const GEVector<GEString>& args) { GetShaderLibrary().ReloadShader(GEString(args[0])); }, "reload_shader <name>");
		GetShaderLibrary().AddShader("dnm");

		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.IsSwapchain = true;
			framebufferSpec.clearColor = { 1.0f, 1.0f, 1.0f };
			mem::Ref<Framebuffer> _defaultFramebuffer = Framebuffer::Create(framebufferSpec);
			PipelineSpec pipelineSpec{};
			pipelineSpec.shader = GetShaderLibrary().GetShader("dnm");
			pipelineSpec.targetFramebuffer = _defaultFramebuffer;
			mem::Ref<Pipeline> _defaultPipeline = Pipeline::Create(pipelineSpec);
			g_defaultRenderPass = RenderPass::Create(_defaultPipeline);
		}
	}

	void Renderer3D::Destroy() {
		g_defaultRenderPass = nullptr;
		s_data._shaderLibrary = nullptr;
		g_renderAPI = nullptr;
	}

	void Renderer3D::BeginFrame() {
		g_renderAPI->BeginFrame();
	}
	void Renderer3D::EndFrame() {
		g_renderAPI->EndFrame();
		g_frameIndex = (g_frameIndex + 1) % MaxFramesInFlight;
	}
	void Renderer3D::BeginDefaultPass() {
		g_defaultRenderPass->Begin();
	}
	void Renderer3D::EndDefaultPass() {
		g_defaultRenderPass->End();
	}
	void Renderer3D::DrawVertex() {
		g_renderAPI->DrawVertex();
	}
	void Renderer3D::DrawIndexed() {
		g_renderAPI->DrawIndexed();
	}

	uint32_t Renderer3D::GetFrameIndex() { return g_frameIndex; }
	RenderStats Renderer3D::GetRenderStats() { return g_renderAPI->GetRenderStats(); }
	ShaderLibrary& Renderer3D::GetShaderLibrary() { return *s_data._shaderLibrary; }
	ge::mem::Ref<RenderPass> Renderer3D::GetDefaultRenderPass() { return g_defaultRenderPass; }
	mem::Ref<RenderAPI> Renderer3D::GetRenderAPI() { return g_renderAPI; }
}