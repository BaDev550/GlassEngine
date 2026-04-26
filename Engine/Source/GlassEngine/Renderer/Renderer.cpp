#include "Renderer.h"
#include "GlassEngine/Renderer/Texture.h"
#include "GlassEngine/Renderer/Sampler.h"
#include "ShaderLibrary.h"

namespace ge::renderer {
	struct RenderData {
		mem::Scope<ShaderLibrary> _shaderLibrary = nullptr;
		mem::Ref<Sampler> _defaultSampler = nullptr;
		mem::Ref<Texture2D> _defaultWhiteTexture = nullptr;
		mem::Ref<Texture2D> _defaultBlackTexture = nullptr;
		mem::Ref<Texture2D> _defaultNormal = nullptr;
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
			FramebufferSpec framebufferSpec{};
			framebufferSpec.Attachments = { FramebufferAttachment(ImageFormat::D32S8) };
			framebufferSpec.IsSwapchain = true;
			mem::Ref<Framebuffer> _defaultFramebuffer = Framebuffer::Create(framebufferSpec);
			g_defaultRenderPass = RenderPass::Create(_defaultFramebuffer, "DEFAULT_PASS");
			s_data._defaultSampler = Sampler::Create(SamplerSpec{});
		}
	}

	void Renderer3D::Destroy() {
		g_defaultRenderPass = nullptr;
		s_data._shaderLibrary = nullptr;
		s_data._defaultSampler = nullptr;
		s_data._defaultWhiteTexture = nullptr;
		s_data._defaultBlackTexture = nullptr;
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

	void Renderer3D::DrawVertex(uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex, uint32_t firstInstance) {
		g_renderAPI->Draw(vertexCount, instanceCount, vertexBuffer, firstVertex, firstInstance);
	}
	void Renderer3D::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex, uint32_t firstInstance, int32_t vertexOffset) {
		g_renderAPI->DrawIndexed(indexCount, instanceCount, vertexBuffer, indexBuffer, firstIndex, firstInstance, vertexOffset);
	}
	void Renderer3D::DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex, ge::mem::Ref<MaterialTable> materialTable, const glm::mat4& transform) {
		g_renderAPI->DrawStaticMesh(pipeline, mesh, lodIndex, materialTable, transform);
	}

	uint32_t Renderer3D::GetFrameIndex() { return g_frameIndex; }
	RenderStats Renderer3D::GetRenderStats() { return g_renderAPI->GetRenderStats(); }
	ShaderLibrary& Renderer3D::GetShaderLibrary() { return *s_data._shaderLibrary; }
	ge::mem::Ref<RenderPass> Renderer3D::GetDefaultRenderPass() { return g_defaultRenderPass; }
	mem::Ref<RenderAPI> Renderer3D::GetRenderAPI() { return g_renderAPI; }
}