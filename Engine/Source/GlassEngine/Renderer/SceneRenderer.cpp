#include "SceneRenderer.h"
#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Renderer/Pipeline.h"
#include "GlassEngine/Renderer/Types.h"
#include "GlassEngine/Scene/Scene.h"
#include "ShaderLibrary.h"
#include "Renderer.h"

namespace ge::renderer {
	SceneRenderer::SceneRenderer(Scene* scene) : _scene(scene) {
		// framebuffer + Render pass
		{
			FramebufferSpec fspec{};
			fspec.attachments = { 
				FramebufferAttachment{ImageFormat::R10G10B10A2Unorm}, // surface 
				FramebufferAttachment{ImageFormat::RGBA8Unorm}, // albedo
				FramebufferAttachment{ImageFormat::RGBA8Unorm}, // emissive, instensity
				FramebufferAttachment{ImageFormat::RG8Unorm}, // metallic roughness
				FramebufferAttachment{ImageFormat::D32S8} 
			};
			fspec.attachments[1].clearValue = { 1.f, 0 };
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
		}

		// Scene Data
		{
			BufferSpec spec{};
			spec.cpuAccess = BufferCpuAccess::Write;
			spec.elementSize = sizeof(SceneData);
			spec.elementCount = 1;
			spec.memoryType = BufferMemoryType::DeviceMemory;
			spec.usageFlags = BufferUsageFlagsBits::Readonly;
			_sceneDataBuffer = Buffer::Create(spec);
			_data = _sceneDataBuffer->GetMappedPtr<SceneData>();
			Engine::Get().GetApplicationWindow().GetRenderContext().SetUniformBuffer(_sceneDataBuffer, 0);
		}

		// Light buffers
		{
			// Point light buffer
			{
				BufferSpec spec{};
				spec.cpuAccess = BufferCpuAccess::Write;
				spec.elementSize = sizeof(PointLight);
				spec.elementCount = GE_MAX_POINT_LIGHT_COUNT;
				spec.memoryType = BufferMemoryType::DeviceMemory;
				spec.usageFlags = BufferUsageFlagsBits::Readonly;
				_pointLightsBuffer = Buffer::Create(spec);
				_data->pointLightBufferGPUAddress = _pointLightsBuffer->GetGPUAddress();
			}
			// Spot light buffer
			{
				BufferSpec spec{};
				spec.cpuAccess = BufferCpuAccess::Write;
				spec.elementSize = sizeof(SpotLight);
				spec.elementCount = GE_MAX_SPOT_LIGHT_COUNT;
				spec.memoryType = BufferMemoryType::DeviceMemory;
				spec.usageFlags = BufferUsageFlagsBits::Readonly;
				_spotLightBuffer = Buffer::Create(spec);
				_data->spotLightBufferGPUAddress = _spotLightBuffer->GetGPUAddress();
			}
			// Directional light buffer
			{
				BufferSpec spec{};
				spec.cpuAccess = BufferCpuAccess::Write;
				spec.elementSize = sizeof(DirectionalLight);
				spec.elementCount = 1;
				spec.memoryType = BufferMemoryType::DeviceMemory;
				spec.usageFlags = BufferUsageFlagsBits::Readonly;
				_directionalLightBuffer = Buffer::Create(spec);
				_data->directionalLightBufferGPUAddress = _directionalLightBuffer->GetGPUAddress();
			}
		}

		_endlessGrid = mem::Ref<EndlessGrid>::Create(_framebuffer);
	}

	void SceneRenderer::Resize(uint32_t width, uint32_t height) {
		_framebuffer->Resize(width, height);
	}

	SceneRenderer::~SceneRenderer() {}

	void SceneRenderer::DrawScene(const ge::mem::Ref<Camera>& camera)
	{
		_data->cameraData.view = camera->GetView();
		_data->cameraData.proj = camera->GetProjection();
		_data->cameraData.pos = camera->GetPosition();
		
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