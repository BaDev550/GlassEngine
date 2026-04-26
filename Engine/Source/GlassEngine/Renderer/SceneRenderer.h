#pragma once
#include "GlassEngine/Core/Core.h"
#include "RenderPass.h"
#include "Camera.h"

namespace ge { class Scene; }
namespace ge::renderer {
	class SceneRenderer : public ge::mem::RefCounted {
	public:
		SceneRenderer(Scene* scene);
		~SceneRenderer();

		void DrawScene(const ge::mem::Ref<Camera>& camera);
	private:
		Scene* _scene;

		ge::mem::Ref<Framebuffer> _framebuffer;
		ge::mem::Ref<Pipeline> _pipeline;

		struct CameraData {
			glm::mat4 proj;
			glm::mat4 view;
		} _cameraData;
		ge::mem::Ref<Buffer> _cameraBuffer;
	};
}