#pragma once
#include "GlassEngine/Core/Core.h"
#include "RenderPass.h"
#include "Camera.h"
#include "EndlessGrid.h"

namespace ge { class Scene; }
namespace ge::renderer {
	class SceneRenderer : public ge::mem::RefCounted {
	public:
		SceneRenderer(Scene* scene);
		~SceneRenderer();

		ge::mem::Ref<Framebuffer>& GetOffscreenFramebuffer() { return _framebuffer; }
		void DrawScene(const ge::mem::Ref<Camera>& camera);
		void Resize(uint32_t width, uint32_t height);
	private:
		Scene* _scene;

		ge::mem::Ref<Framebuffer> _framebuffer;
		ge::mem::Ref<Pipeline> _pipeline;
		ge::mem::Ref<RenderPass> _renderPass;

		struct CameraData {
			glm::mat4 proj;
			glm::mat4 view;
			glm::vec3 pos;
		} _cameraData;
		ge::mem::Ref<Buffer> _cameraBuffer;
		ge::mem::Ref<EndlessGrid> _endlessGrid;
	};
}