#include "Renderer.h"

namespace ge::renderer {
	struct RenderData {
		mem::Scope<ShaderLibrary> _shaderLibrary = nullptr;
	} static s_data;
	static mem::Ref<RenderAPI> g_renderAPI = nullptr;

	void Renderer3D::Init() {
		g_renderAPI = RenderAPI::Create();

		//GetShaderLibrary().AddShader("", "", ""); Adding shaders
	}

	void Renderer3D::Destroy() {
		s_data._shaderLibrary = nullptr;
		g_renderAPI = nullptr;
	}

	void Renderer3D::BeginFrame() {
		g_renderAPI->BeginFrame();
	}
	void Renderer3D::EndFrame() {
		g_renderAPI->EndFrame();
	}
	void Renderer3D::BeginDefaultPass() {
		g_renderAPI->BeginDefaultPass();
	}
	void Renderer3D::EndDefaultPass() {
		g_renderAPI->EndDefaultPass();
	}
	void Renderer3D::DrawVertex() {
		g_renderAPI->DrawVertex();
	}
	void Renderer3D::DrawIndexed() {
		g_renderAPI->DrawIndexed();
	}

	uint32_t Renderer3D::GetFrameIndex()
	{
		return 0;
	}

	RenderStats Renderer3D::GetRenderStats() { return g_renderAPI->GetRenderStats(); }
	ShaderLibrary& Renderer3D::GetShaderLibrary() { return *s_data._shaderLibrary; }
	mem::Ref<RenderAPI> Renderer3D::GetRenderAPI() { return g_renderAPI; }
}