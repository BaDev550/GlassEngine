#include "SceneRenderer.h"
#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Renderer/Pipeline.h"
#include "GlassEngine/Renderer/Types.h"
#include "GlassEngine/Scene/Scene.h"
#include "ShaderLibrary.h"
#include "Renderer.h"

namespace ge::renderer {
	SceneRenderer::SceneRenderer(Scene* scene) : _scene(scene) {
		FramebufferSpec fspec{};
		fspec.attachments = { FramebufferAttachment{ImageFormat::RGBA8Srgb}, FramebufferAttachment{ImageFormat::D32S8} };
		fspec.attachments[1].clearValue = {1.f, 0};
		fspec.width = Engine::Get().GetApplicationWindow().GetWidth();
		fspec.height = Engine::Get().GetApplicationWindow().GetHeight();
		_framebuffer = Framebuffer::Create(fspec);
		_renderPass = RenderPass::Create(_framebuffer, "SCENE_RENDER_PASS");

		PipelineSpec spec{};
		spec.inputAssemblySpec.vertexAttributes = {
			VertexAttribute{VertexFormat::RGBA32Float, 0, 0, 0, },
			VertexAttribute{VertexFormat::RGBA32Float, 16, 1, 0, },
			VertexAttribute{VertexFormat::RG32Float, 32, 2, 0, }
		};
		spec.inputAssemblySpec.vertexBindings = {
			VertexBinding{40, 0, VertexInputRate::Vertex}
		};
		spec.depthStencilSpec.depthTestEnable = true;
		spec.depthStencilSpec.depthWriteEnable = true;
		spec.depthStencilSpec.depthTestCompareOp = CompareOp::Less;
		spec.resterizerSpec.cullMode = CullMode::Back;
		spec.shader = Renderer3D::GetShaderLibrary().GetShader("dnm");
		spec.targetFramebuffer = _framebuffer;
		_pipeline = Pipeline::Create(spec);

		// Camera buffer
		{
			BufferSpec spec{};
			spec.cpuAccess = BufferCpuAccess::Write;
			spec.elementSize = sizeof(CameraData);
			spec.elementCount = 1;
			spec.memoryType = BufferMemoryType::DeviceMemory;
			spec.usageFlags = BufferUsageFlagsBits::Uniform;
			_cameraBuffer = Buffer::Create(spec);
			std::memcpy(_cameraBuffer->GetMappedPtr(), &_cameraData, sizeof(CameraData));
			Engine::Get().GetApplicationWindow().GetRenderContext().SetUniformBuffer(_cameraBuffer, 0);
		}

		_endlessGrid = mem::Ref<EndlessGrid>::Create(_framebuffer);
	}

	SceneRenderer::~SceneRenderer() {}

	void SceneRenderer::DrawScene(const ge::mem::Ref<Camera>& camera)
	{
		_cameraData.view = camera->GetView();
		_cameraData.proj = camera->GetProjection();
		_cameraData.pos = camera->GetPosition();
		std::memcpy(_cameraBuffer->GetMappedPtr(), &_cameraData, sizeof(CameraData));

		_renderPass->Begin();
		_endlessGrid->Draw();

		auto view = _scene->GetRegistry().view<const TransformComponent, StaticMeshComponent>();
		view.each([&](const TransformComponent& tc, StaticMeshComponent& smc) {
			if (smc.isVisible) {
				auto staticMesh = AssetManager::GetAsset<StaticMesh>(smc.meshHandle);
				if (staticMesh) {
					const auto& lods = staticMesh->GetLODs();
					uint32_t lodIndex = 0;
					if (smc.calculateLOD) {
						lodIndex = staticMesh->GetLODManager().GetLODindex(lods, camera, tc.position);
						smc.lodLevel = lodIndex;
					}
					else {
						lodIndex = smc.lodLevel;
					}
					if (lodIndex >= lods.size()) {
						lodIndex = lods.empty() ? 0 : lods.size() - 1;
					}

					Renderer3D::DrawStaticMesh(_pipeline, staticMesh, lodIndex, smc.materialTable, tc.Mat4());
				}
			}
			});

		_renderPass->End();
	}
}