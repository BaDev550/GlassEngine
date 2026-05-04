#pragma once
#include "GlassEngine/Core/Core.h"
#include "RenderPass.h"
#include "Texture.h"
#include "Camera.h"
#include "Light.h"
#include "EndlessGrid.h"

namespace ge { class Scene; }
namespace ge::renderer {
	class SceneRenderer : public ge::mem::RefCounted {
	public:
		SceneRenderer(Scene* scene);
		~SceneRenderer();

		ge::mem::Ref<Framebuffer>& GetOffscreenFramebuffer() { return _viewportFramebuffer; }
		void DrawScene(const ge::mem::Ref<Camera>& camera);
		void Resize(uint32_t width, uint32_t height);
	private:
		Scene* _scene;

		ge::mem::Ref<Framebuffer> _gBuffer;
		ge::mem::Ref<Pipeline> _geometryPassPipeline;
		ge::mem::Ref<RenderPass> _geometryPass;

		ge::mem::Ref<Framebuffer> _viewportFramebuffer;
		ge::mem::Ref<Pipeline> _lightingPassPipeline;
		ge::mem::Ref<RenderPass> _lightingPass;

		ge::mem::Ref<EndlessGrid> _endlessGrid;

		struct LightEnviromentData {
			mem::Ref<Texture2D> LUTtexture;
			GEVector<PointLight> pointLights;
			GEVector<SpotLight> spotLights;
			DirectionalLight directionalLight;
		} _lightEnviromentData;

		struct SceneData {
			struct alignas(16) CameraData {
				glm::mat4 proj;
				glm::mat4 view;
				glm::vec3 pos;
			} cameraData;
			
			uint64_t pointLightBufferGPUAddress;
			uint64_t spotLightBufferGPUAddress;
			uint64_t directionalLightBufferGPUAddress;

			uint32_t lutIndex;
			uint32_t enviromentMapIndex;
			
			uint32_t gBufferNormal;
			uint32_t gBufferAlbedoMetallic;
			uint32_t gBufferEmissive;
			uint32_t gBufferRoughness;
			uint32_t gBufferEntityId;

			uint32_t pointLightCount;
			uint32_t spotLightCount;
		} *_data;
		ge::mem::Ref<Buffer> _sceneDataBuffer;
		ge::mem::Ref<Buffer> _pointLightsBuffer;
		ge::mem::Ref<Buffer> _spotLightBuffer;
		ge::mem::Ref<Buffer> _directionalLightBuffer;
	};
}