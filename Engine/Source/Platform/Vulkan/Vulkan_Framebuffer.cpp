#include "gepch.h"
#include "GlassEngine/Renderer/Types.h"
#include "Vulkan_Framebuffer.h"

namespace ge::renderer {
	Vulkan_Framebuffer::Vulkan_Framebuffer(const FramebufferSpec& spec) {
		_specs = spec;
		Invalidate(spec);
	}

	void Vulkan_Framebuffer::Invalidate(const FramebufferSpec& spec)
	{
		_colorAttachments.clear();
		if (!_specs.IsSwapchain) {
			for (const auto& attachment : spec.Attachments) {
				if (attachment.existingImage) {
					if (utility::IsDepthStencilFormat(attachment.existingImage->GetSpec().imageFormat)) {
						_depthStencilAttachment = attachment.existingImage;
					}
					else {
						_colorAttachments.emplace_back(attachment.existingImage);
					}
					continue;
				}
				ImageSpec imageSpec{};
				imageSpec.imageFormat = attachment.Format;
				imageSpec.extent.x = spec.width;
				imageSpec.extent.y = spec.height;
				if (utility::IsDepthStencilFormat(attachment.Format)) {
					imageSpec.usageFlags |= ImageUsageFlagsBits::DepthStencilAttachment;
					_depthStencilAttachment = Image::Create(imageSpec);
				}
				else {
					imageSpec.usageFlags |= ImageUsageFlagsBits::ColorAttachment;
					_colorAttachments.emplace_back(Image::Create(imageSpec));
				}
			}
		}
		else {
			if (utility::IsDepthStencilFormat(spec.Attachments[0].Format)) {
				ImageSpec imageSpec{};
				imageSpec.imageFormat = spec.Attachments[0].Format;
				imageSpec.extent.x = Application::Get()->GetWindow().GetWidth();
				imageSpec.extent.y = Application::Get()->GetWindow().GetHeight();
				imageSpec.extent.z = 1.0f;
				imageSpec.usageFlags |= ImageUsageFlagsBits::DepthStencilAttachment;
				_depthStencilAttachment = Image::Create(imageSpec);
			}
		}
	}

	void Vulkan_Framebuffer::Resize(uint32_t width, uint32_t height)
	{
	}
}