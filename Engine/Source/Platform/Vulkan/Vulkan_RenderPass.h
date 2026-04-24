#pragma once
#include "GlassEngine/Renderer/RenderPass.h"
namespace ge::renderer {
	class Vulkan_RenderPass : public RenderPass {
	public:
		Vulkan_RenderPass(const ge::mem::Ref<Framebuffer>& framebuffer, std::string_view debugName);
		virtual void Begin(uint32_t layer = 0) override;
		virtual void End(uint32_t layer = 0) override;
	};
}