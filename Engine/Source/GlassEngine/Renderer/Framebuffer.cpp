#include "gepch.h"
#include "Framebuffer.h"

namespace ge::renderer {
	Framebuffer::Framebuffer(const FramebufferSpec& spec) {
		Invalidate(spec);
	}

	void Framebuffer::Invalidate(const FramebufferSpec& spec) {
		_specs = spec;
		_colorAttachments.clear();
		_hasSwapchainImage = false;

		for (auto& attachment : _specs.attachments) {
			if (attachment.isSwapchain) {
				_hasSwapchainImage = true;
				continue;
			}
			// TODO (0x): attachment.existingImage not resizing 
			// else if (attachment.existingImage) {
			// 	attachment.format = attachment.existingImage->GetSpecRef().imageFormat;

			// 	if (utility::IsDepthFormat(attachment.existingImage->GetSpec().imageFormat) 
			// 	|| utility::IsDepthStencilFormat(attachment.existingImage->GetSpec().imageFormat)) {
			// 		_depthStencilAttachment = attachment.existingImage;
			// 	}
			// 	else {
			// 		_colorAttachments.emplace_back(attachment.existingImage);
			// 	}
			// 	continue;
			// }

			ImageSpec imageSpec{};
			imageSpec.imageFormat = attachment.format;
			imageSpec.extent.x = spec.width;
			imageSpec.extent.y = spec.height;
			imageSpec.extent.z = 1;
			if (utility::IsDepthFormat(attachment.format) 
			|| utility::IsDepthStencilFormat(attachment.format)) {
				imageSpec.usageFlags |= ImageUsageFlagsBits::DepthStencilAttachment;
				_depthStencilAttachment = Image::Create(imageSpec);
				attachment.existingImage = _depthStencilAttachment;
				attachment.existingImage->SetDebugName("framebuffer depth stencil attachment");
			}
			else {
				imageSpec.usageFlags |= ImageUsageFlagsBits::ColorAttachment;
				imageSpec.usageFlags |= ImageUsageFlagsBits::Readonly;
				attachment.existingImage = _colorAttachments.emplace_back(Image::Create(imageSpec));
				attachment.existingImage->SetDebugName("framebuffer color attachment");
			}
		}
	}

	void Framebuffer::Resize(uint32_t width, uint32_t height) {
		_specs.height = height;
		_specs.width = width;
		Invalidate(_specs);
	}
}