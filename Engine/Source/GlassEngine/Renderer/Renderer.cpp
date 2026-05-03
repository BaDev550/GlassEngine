#include "Renderer.h"
#include "GlassEngine/Renderer/Buffer.h"
#include "GlassEngine/Renderer/Texture.h"
#include "GlassEngine/Renderer/Sampler.h"
#include "GlassEngine/Renderer/Types.h"
#include "ShaderLibrary.h"

namespace ge::renderer {
	struct RenderData {
		mem::Scope<ShaderLibrary> _shaderLibrary = nullptr;
		mem::Ref<Sampler> _defaultSampler = nullptr;
		mem::Ref<Texture2D> _defaultWhiteTexture = nullptr;
		mem::Ref<Texture2D> _defaultBlackTexture = nullptr;
		mem::Ref<Buffer> _defaultsUbo = nullptr;
	} static s_data;

	static mem::Ref<RenderAPI> g_renderAPI = nullptr;
	static uint32_t g_frameIndex = 0;
	static RenderConfig g_config;
	static RenderCommandQueue g_renderCommandQueue;

	void Renderer3D::Init() {
		GE_PROFILE_SCOPE("Renderer3D::Init");

		g_renderAPI = RenderAPI::Create();
		s_data._shaderLibrary = mem::CreateScope<ShaderLibrary>();

		GE_ADD_CONSOLE_COMMAND("r", "wireframe", [](const GEVector<GEString>& args) { g_config.wireframe = (args[0] == "1" || args[0] == "true");}, "wireframe <enabled>");
		GE_ADD_CONSOLE_COMMAND("r", "vsync", [](const GEVector<GEString>& args) {g_config.vsync = (args[0] == "1" || args[0] == "true");}, "vsync <enabled>");

		GE_ADD_CONSOLE_COMMAND("r", "reload_shaders", [](const GEVector<GEString>& args) { GetShaderLibrary().ReloadAll(); });
		GE_ADD_CONSOLE_COMMAND("r", "reload_shader", [](const GEVector<GEString>& args) { GetShaderLibrary().ReloadShader(GEString(args[0])); }, "reload_shader <name>");
		GetShaderLibrary().AddShader("dnm");
		GetShaderLibrary().AddShader("grid");

		{
			TextureSpec whiteTextureData{};
			constexpr uint32_t whiteData = 0xffffffff;
			whiteTextureData.width = 1;
			whiteTextureData.height = 1;
			whiteTextureData.format = ImageFormat::RGBA8;
			s_data._defaultWhiteTexture = Texture2D::Create(whiteTextureData, &whiteData);
			s_data._defaultWhiteTexture->SetDebugName("White Texture");

			s_data._defaultSampler = Sampler::Create(SamplerSpec{});
			s_data._defaultSampler->SetDebugName("Default Sampler");
			
			TextureSpec blackTextureData{};
			constexpr uint32_t blackData = 0x00000000;
			blackTextureData.width = 1;
			blackTextureData.height = 1;
			blackTextureData.format = ImageFormat::RGBA8;
			s_data._defaultBlackTexture = Texture2D::Create(blackTextureData, &blackData);
			s_data._defaultBlackTexture->SetDebugName("Black Texture");

			struct Defaults {
				uint32_t defaultSampler = s_data._defaultSampler->GetHandle();
				uint32_t whiteTexture = s_data._defaultWhiteTexture->GetHandle();
				uint32_t blackTexture = s_data._defaultBlackTexture->GetHandle();
			} const defaults{};

			BufferSpec defaultsUboSpec{};
			defaultsUboSpec.cpuAccess = BufferCpuAccess::Write;
			defaultsUboSpec.memoryType = BufferMemoryType::DeviceMemory;
			defaultsUboSpec.usageFlags = BufferUsageFlagsBits::Uniform;
			defaultsUboSpec.elementSize = sizeof(defaults);
			defaultsUboSpec.elementCount = 1;
			s_data._defaultsUbo = Buffer::Create(defaultsUboSpec);
			s_data._defaultsUbo->SetDebugName("Defaults Buffer");

			*s_data._defaultsUbo->GetMappedPtr<Defaults>() = defaults;

			Engine::Get().GetApplicationWindow().GetRenderContext().SetUniformBuffer(s_data._defaultsUbo, 2);
		}
	}

	void Renderer3D::Destroy() {
		s_data._shaderLibrary = nullptr;
		s_data._defaultSampler = nullptr;
		s_data._defaultWhiteTexture = nullptr;
		s_data._defaultBlackTexture = nullptr;
		s_data._defaultsUbo = nullptr;
		g_renderAPI = nullptr;
	}

	// TODO (0x): TEMP
	void Renderer3D::EndFrame() {
		// g_renderAPI->EndFrame();
		g_frameIndex = (g_frameIndex + 1) % MaxFramesInFlight;
	}

	void Renderer3D::DrawVertex(ge::mem::Ref<Pipeline>& pipeline, uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex, uint32_t firstInstance) {
		g_renderAPI->Draw(pipeline, vertexCount, instanceCount, vertexBuffer, firstVertex, firstInstance);
	}
	void Renderer3D::DrawIndexed(ge::mem::Ref<Pipeline>& pipeline, uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex, uint32_t firstInstance, int32_t vertexOffset) {
		g_renderAPI->DrawIndexed(pipeline, indexCount, instanceCount, vertexBuffer, indexBuffer, firstIndex, firstInstance, vertexOffset);
	}
	void Renderer3D::DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex, ge::mem::Ref<MaterialTable> materialTable, const glm::mat4& transform) {
		g_renderAPI->DrawStaticMesh(pipeline, mesh, lodIndex, materialTable, transform);
	}
	void Renderer3D::WaitAndRender() {
		g_renderCommandQueue.Execute();
	}

	ImTextureID Renderer3D::GetImGuiTexture(const GEString& name, ge::mem::Ref<Image>& image) {
		return g_renderAPI->GetImGuiTexture(name, image, ImageSubresource{}, s_data._defaultSampler);
	}

	void Renderer3D::Submit(std::function<void()> func) { g_renderCommandQueue.Submit(func); }
	uint32_t Renderer3D::GetFrameIndex() { return g_frameIndex; }
	RenderConfig Renderer3D::GetRenderConfig() { return g_config; }
	RenderStats Renderer3D::GetRenderStats() { return g_renderAPI->GetRenderStats(); }
	ShaderLibrary& Renderer3D::GetShaderLibrary() { return *s_data._shaderLibrary; }
	ge::mem::Ref<Texture2D>& Renderer3D::GetWhiteTexture() { return s_data._defaultWhiteTexture; }
	mem::Ref<RenderAPI> Renderer3D::GetRenderAPI() { return g_renderAPI; }
}