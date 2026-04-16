#include "gepch.h"
#include "GlassEngine/Renderer/Types.h"
#include "Vulkan_Framebuffer.h"

namespace ge::renderer {
	Vulkan_Framebuffer::Vulkan_Framebuffer(const FramebufferSpecification& spec) {
		_specs = spec;
		Invalidate(spec);
	}

	void Vulkan_Framebuffer::Invalidate(const FramebufferSpecification& spec)
	{
		_colorAttachments.clear();
		_depthAttachments.clear();
		if (!_specs.IsSwapchain) {
			for (const auto& attachment : spec.Attachments.Attachments) {
				TextureSpec textureSpec{};
				textureSpec.format = attachment;
				textureSpec.width = spec.width;
				textureSpec.height = spec.height;
				textureSpec.attachment = true;
				if (utility::IsDepthFormat(attachment)) {
					_depthAttachments.emplace_back(Texture2D::Create(textureSpec));
				}
				else {
					_colorAttachments.emplace_back(Texture2D::Create(textureSpec));
				}
			}
		} // if it is swapchain we got image views from swapchain
	}

	void Vulkan_Framebuffer::Resize(uint32_t width, uint32_t height)
	{
	}
}