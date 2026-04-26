#include "SceneRenderer.h"
#include "GlassEngine/Renderer/Pipeline.h"
#include "GlassEngine/Renderer/Types.h"
#include "GlassEngine/Scene/Scene.h"
#include "ShaderLibrary.h"
#include "Renderer.h"

namespace ge::renderer {
	SceneRenderer::SceneRenderer(Scene* scene) : _scene(scene) {
		FramebufferSpec fspec{};
		fspec.IsSwapchain = true;
		_framebuffer = Framebuffer::Create(fspec);

		PipelineSpec spec{};
		spec.inputAssemblySpec.vertexAttributes = {
			VertexAttribute{VertexFormat::RGBA32Float, 0, 0, 0, },
			VertexAttribute{VertexFormat::RGBA32Float, 16, 1, 0, },
			VertexAttribute{VertexFormat::RG32Float, 32, 2, 0, }
		};
		spec.inputAssemblySpec.vertexBindings = {
			VertexBinding{40, 0, VertexInputRate::Vertex}
		};
		spec.shader = Renderer3D::GetShaderLibrary().GetShader("dnm");
		spec.targetFramebuffer = _framebuffer;
		_pipeline = Pipeline::Create(spec);
	}
	SceneRenderer::~SceneRenderer() {}

	void SceneRenderer::DrawScene(ge::mem::Ref<Camera>& camera)
	{
		Renderer3D::BeginDefaultPass();
		auto view = _scene->GetRegistry().view<const TransformComponent, StaticMeshComponent>();
		view.each([&](const TransformComponent& tc, StaticMeshComponent& smc) {
			if (smc.isVisible) {
				auto staticMesh = Application::Get()->GetAssetManager()->GetAsset(smc.meshHandle).Cast<StaticMesh>();
				uint32_t lodIndex = staticMesh->GetLODManager().GetLODindex(staticMesh->GetLODs(), camera, tc.position);
				Renderer3D::DrawStaticMesh(_pipeline, staticMesh, smc.lodLevel, smc.materialTable, tc.Mat4());
			}
			});
		Renderer3D::EndDefaultPass();
	}
}