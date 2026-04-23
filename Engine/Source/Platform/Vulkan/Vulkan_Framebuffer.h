#pragma once
#include "GlassEngine/Renderer/Framebuffer.h"

namespace ge::renderer {
	class Vulkan_Framebuffer : public Framebuffer {
	public:
		Vulkan_Framebuffer(const FramebufferSpec& spec);
		virtual void Invalidate(const FramebufferSpec& spec) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
	};
}