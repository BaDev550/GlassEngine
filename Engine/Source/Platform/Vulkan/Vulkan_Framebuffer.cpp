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
				ImageSpec imageSpec{};
				imageSpec.imageFormat = attachment;
				imageSpec.extent.x = spec.width;
				imageSpec.extent.y = spec.height;
				if (utility::IsDepthFormat(attachment)) {
					imageSpec.usageFlags = ImageUsageFlagsBits::DepthStencilAttachment;
					_depthAttachments.emplace_back(Image::Create(imageSpec));
				}
				else {
					imageSpec.usageFlags = ImageUsageFlagsBits::ColorAttachment;
					_colorAttachments.emplace_back(Image::Create(imageSpec));
				}
			}
		} // if it is swapchain we got image views from swapchain
	}

	void Vulkan_Framebuffer::Resize(uint32_t width, uint32_t height)
	{
	}
}