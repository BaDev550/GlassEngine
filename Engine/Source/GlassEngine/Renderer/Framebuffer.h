#pragma once
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Memory/Ref.h"
#include <cstdint>
#include <initializer_list>
#include "Types.h"

namespace ge::renderer {
	struct FramebufferAttachment {
		FramebufferAttachment() = default;
		FramebufferAttachment(ImageFormat format) : format(format) {}
		FramebufferAttachment(bool isSwapchain) : isSwapchain(isSwapchain) {}
		FramebufferAttachment(ge::mem::Ref<Image> existingImage, ImageSubresource subresource) 
			: existingImage(existingImage), subresource(subresource) {}

		AttachmentLoadOp loadOp = AttachmentLoadOp::Clear;
		AttachmentStoreOp storeOp = AttachmentStoreOp::Store;
		ClearValue clearValue{};

		ge::mem::Ref<Image> existingImage = nullptr;
		ImageSubresource subresource{};

		ImageFormat format = ImageFormat::RGBA8;

		bool isSwapchain = false;
	};

	struct FramebufferSpec {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t layers = 0;
		GEVector<FramebufferAttachment> attachments;
	};

	class Framebuffer : public ge::mem::RefCounted {
	public:
		Framebuffer(const FramebufferSpec& spec);
		void Invalidate(const FramebufferSpec& spec);
		void Resize(uint32_t width, uint32_t height);
		uint32_t GetWidth() const { return _specs.width; }
		uint32_t GetHeight() const { return _specs.height; }
		uint32_t GetAttachmentCount() const { return static_cast<uint32_t>(_specs.attachments.size()); }
		GEVector<FramebufferAttachment>& GetAttachments() { return _specs.attachments; }
		ge::mem::Ref<Image>& GetColorAttachmentTexture(uint32_t index = 0) { return _colorAttachments[index]; };
		ge::mem::Ref<Image>& GetDepthStencilAttachmentTexture() { return _depthStencilAttachment; };
		bool HasDepthStencilAttachment() const { return _depthStencilAttachment; }
		bool HasSwapchainImage() const { return _hasSwapchainImage; }
		const FramebufferSpec& GetSpecification() const { return _specs; }

		static ge::mem::Ref<Framebuffer> Create(const FramebufferSpec& spec) {
			return ge::mem::Ref<Framebuffer>::Create(spec);
		}
	protected:
		GEVector<ge::mem::Ref<Image>> _colorAttachments;
		ge::mem::Ref<Image> _depthStencilAttachment;
		FramebufferSpec _specs;
		bool _hasSwapchainImage{};
	};
}