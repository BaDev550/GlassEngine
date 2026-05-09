#include "SceneRenderer.h"
#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Core/Engine.h"
#include "GlassEngine/Renderer/Light.h"
#include "GlassEngine/Renderer/Pipeline.h"
#include "GlassEngine/Renderer/Types.h"
#include "GlassEngine/Scene/Scene.h"
#include "ShaderLibrary.h"
#include "Renderer.h"
#include <glm/matrix.hpp>

namespace ge::renderer {
	SceneRenderer::SceneRenderer(Scene* scene) : _scene(scene) {
		// framebuffer + Render pass
		{
			// geometry pass
			{
				FramebufferSpec fspec{};
				fspec.attachments = { 
					FramebufferAttachment{ImageFormat::D32},
					FramebufferAttachment{ImageFormat::R10G10B10A2Unorm}, // surface normal 
					FramebufferAttachment{ImageFormat::RGBA8Unorm}, // albedo, metallic
					FramebufferAttachment{ImageFormat::RGBA8Unorm}, // emissive, instensity
					FramebufferAttachment{ImageFormat::R8Unorm}, // roughness
					FramebufferAttachment{ImageFormat::R32Uint}, // entityId
				};
				fspec.attachments[0].clearValue = { 1.f, 0 };
				fspec.width = Engine::Get().GetApplicationWindow().GetWidth();
				fspec.height = Engine::Get().GetApplicationWindow().GetHeight();
				_gBuffer = Framebuffer::Create(fspec);
				_geometryPass = RenderPass::Create(_gBuffer, "SCENE_GEOMETRY_PASS");

				PipelineSpec spec{};
				spec.inputAssemblySpec.vertexAttributes = {
					VertexAttribute{VertexFormat::RGBA32Float, 0, 0, 0, },
					VertexAttribute{VertexFormat::RGBA32Float, 16, 1, 0, },
					VertexAttribute{VertexFormat::RGBA32Float, 32, 2, 0, },
					VertexAttribute{VertexFormat::RG32Float, 48, 3, 0, }
				};
				spec.inputAssemblySpec.vertexBindings = {
					// TODO(dnm): maybe align 16? (need test for performance)
					VertexBinding{56, 0, VertexInputRate::Vertex}
				};

				// PipelineSpec spec{};
				// spec.inputAssemblySpec.vertexAttributes = {
				// 	VertexAttribute{VertexFormat::RGBA32Float /*r10g10b10a2Unorm*/, 0, 0, 0, }, // normal, size 4 bayt
				// 	VertexAttribute{VertexFormat::RGBA32Float /*r10g10b10a2Unorm*/, 4, 1, 0, }, // tangent, size 4 bayt
				// 	VertexAttribute{VertexFormat::RGBA32Float /*rgba8Unorm*/, 8, 2, 0, }, // color, size 4 bayt
				// 	VertexAttribute{VertexFormat::RG32Float /*rg16Unorm*/, 12, 3, 0, }, // uv, size 4 bayt

				// 	VertexAttribute{VertexFormat::RGBA32Float /*rgba16f*/, 0, 0, 1, }, // pos, 8 bayt
				// 	VertexAttribute{VertexFormat::RGBA32Float /*rgba8Unorm*/, 8, 1, 1, }, // weight, 4 bayt
				// 	VertexAttribute{VertexFormat::RGBA32Float /*rgba8uint*/, 12, 2, 1, }, // joints, 4 bayt
				// };
				// spec.inputAssemblySpec.vertexBindings = {
				// 	// TODO(dnm): maybe align 16? (need test for performance)
				// 	VertexBinding{16, 0, VertexInputRate::Vertex},
				// 	VertexBinding{16, 1, VertexInputRate::Vertex},
				// };
				spec.depthStencilSpec.depthTestEnable = true;
				spec.depthStencilSpec.depthWriteEnable = true;
				spec.depthStencilSpec.depthTestCompareOp = CompareOp::Less;
				spec.resterizerSpec.cullMode = CullMode::Back;
				spec.shader = Renderer3D::GetShaderLibrary().GetShader("GeometryPass");
				spec.targetFramebuffer = _gBuffer;
				_geometryPassPipeline = Pipeline::Create(spec);
			}

			// lighting pass
			{
				FramebufferSpec fspec{};
				fspec.attachments = { FramebufferAttachment{ImageFormat::RGBA8Unorm} };
				fspec.width = Engine::Get().GetApplicationWindow().GetWidth();
				fspec.height = Engine::Get().GetApplicationWindow().GetHeight();
				_viewportFramebuffer = Framebuffer::Create(fspec);
				_lightingPass = RenderPass::Create(_viewportFramebuffer, "SCENE_LIGHTING_PASS");

				PipelineSpec spec{};
				spec.depthStencilSpec.depthTestEnable = false;
				spec.depthStencilSpec.depthWriteEnable = false;
				spec.depthStencilSpec.depthTestCompareOp = CompareOp::None;
				spec.resterizerSpec.cullMode = CullMode::None;
				spec.shader = Renderer3D::GetShaderLibrary().GetShader("LightingPass");
				spec.targetFramebuffer = _viewportFramebuffer;
				_lightingPassPipeline = Pipeline::Create(spec);
			}
		}

		// Scene Data
		{
			BufferSpec spec{};
			spec.cpuAccess = BufferCpuAccess::Write;
			spec.elementSize = sizeof(SceneData);
			spec.elementCount = 1;
			spec.memoryType = BufferMemoryType::DeviceMemory;
			spec.usageFlags = BufferUsageFlagsBits::Uniform;
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
			
			{
				auto &renderContext = Engine::Get().GetApplicationWindow().GetRenderContext();

				_data->gBufferDepth = renderContext.GetReadonlyImageHandle(*_gBuffer->GetDepthStencilAttachmentTexture(), {});
				_data->gBufferNormal = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(0), {});
				_data->gBufferAlbedoMetallic = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(1), {});
				_data->gBufferEmissive = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(2), {});
				_data->gBufferRoughness = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(3), {});
				_data->gBufferEntityId = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(4), {});
			}
		}

		_endlessGrid = mem::Ref<EndlessGrid>::Create(_viewportFramebuffer);
	}

	void SceneRenderer::Resize(uint32_t width, uint32_t height) {
		_viewportFramebuffer->Resize(width, height);
		_gBuffer->Resize(width, height);
		_data->cameraData.extent = {width, height};
		{
			auto &renderContext = Engine::Get().GetApplicationWindow().GetRenderContext();
			renderContext.RemoveReadonlyImageHandle(_data->gBufferDepth);
			renderContext.RemoveReadonlyImageHandle(_data->gBufferNormal);
			renderContext.RemoveReadonlyImageHandle(_data->gBufferAlbedoMetallic);
			renderContext.RemoveReadonlyImageHandle(_data->gBufferEmissive);
			renderContext.RemoveReadonlyImageHandle(_data->gBufferRoughness);
			renderContext.RemoveReadonlyImageHandle(_data->gBufferEntityId);

			_data->gBufferDepth = renderContext.GetReadonlyImageHandle(*_gBuffer->GetDepthStencilAttachmentTexture(), {});
			_data->gBufferNormal = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(0), {});
			_data->gBufferAlbedoMetallic = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(1), {});
			_data->gBufferEmissive = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(2), {});
			_data->gBufferRoughness = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(3), {});
			_data->gBufferEntityId = renderContext.GetReadonlyImageHandle(*_gBuffer->GetColorAttachmentTexture(4), {});
		}
	}

	void SceneRenderer::CollectLightDataFromScene()
	{
		{
			auto group = _scene->GetRegistry().group<PointLightComponent, TransformComponent>();

			auto& pointLights = _lightEnviromentData.pointLights;
			uint32_t pointLightCount = 0;

			pointLights.resize(group.size());
			group.each([&](PointLightComponent& p, const TransformComponent t) {
				p.handle.position = glm::vec4(t.position, 1.0f);
				pointLights[pointLightCount] = p.handle;
				pointLightCount++;
				});

			if (_lightEnviromentData.pointLightCount != group.size() && pointLightCount > 0)
				_data->pointLightCount = pointLightCount;

			_pointLightsBuffer->Write<PointLight>(pointLights.data(), pointLightCount * sizeof(PointLight));
			_lightEnviromentData.pointLightCount = pointLightCount;
		}
		{
			auto view = _scene->GetRegistry().view<DirectionalLightComponent, TransformComponent>();
			auto& directionalLight = _lightEnviromentData.directionalLight;

			view.each([&](DirectionalLightComponent& d, const TransformComponent t) {
				d.handle.direction = glm::vec4(t.rotation, 1.0f);
				directionalLight = d.handle;
				return;
			});

			_data->directionalLight = directionalLight;
		}
	}

	SceneRenderer::~SceneRenderer() {}

	void SceneRenderer::DrawScene(const ge::mem::Ref<Camera>& camera)
	{
		{
			const auto view = camera->GetView();
			const auto proj = camera->GetProjection();
			const auto viewProj = camera->GetProjection() * camera->GetView();
			_data->cameraData.view = view;
			_data->cameraData.proj = proj;
			_data->cameraData.viewProj = viewProj;
			_data->cameraData.invViewProj = glm::inverse(viewProj);
			_data->cameraData.pos = camera->GetPosition();
		}

		CollectLightDataFromScene();

		_geometryPass->Begin();
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
					Renderer3D::DrawStaticMesh(_geometryPassPipeline, staticMesh, lodIndex, smc.materialTable, tc.Mat4());
				}
			}
			});
		_geometryPass->End();

		_lightingPass->Begin();
		Renderer3D::DrawVertex(_lightingPassPipeline, 3, 1, nullptr);
		// _endlessGrid->Draw();
		_lightingPass->End();
	}
}