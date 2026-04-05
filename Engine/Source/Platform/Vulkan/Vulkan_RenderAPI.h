#pragma once
#include "GlassEngine/Renderer/RenderAPI.h"

namespace ge::renderer {
	class Vulkan_RenderAPI : public RenderAPI {
	public:
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void BeginDefaultPass() override;
		virtual void EndDefaultPass() override;
		virtual void DrawVertex(CommandBuffer* cmd) override;
		virtual void DrawIndexed(CommandBuffer* cmd) override;
	};
}