#pragma once
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Memory/Ref.h"
#include <initializer_list>
#include "Types.h"

namespace ge::renderer {
	struct FramebufferAttachment {
		FramebufferAttachment() = default;
		FramebufferAttachment(ImageFormat format) : Format(format) {}
		ImageFormat Format = ImageFormat::RGBA8;
		float depthClearValue = 1.0f;
		glm::vec3 clearColor{ 0.0f };
		ge::mem::Ref<Image> existingImage = nullptr;
	};

	struct FramebufferSpec {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t layers = 0;
		GEVector<FramebufferAttachment> Attachments;
		bool IsSwapchain = false;
	};

	class Framebuffer : public ge::mem::RefCounted {
	public:
		virtual void Invalidate(const FramebufferSpec& spec) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		uint32_t GetWidth() const { return _specs.width; }
		uint32_t GetHeight() const { return _specs.height; }
		uint32_t GetAttachmentCount() const { return static_cast<uint32_t>(_specs.Attachments.size()); }
		GEVector<FramebufferAttachment>& GetAttachments() { return _specs.Attachments; }
		ge::mem::Ref<Image>& GetColorAttachmentTexture(uint32_t index = 0) { return _colorAttachments[index]; };
		ge::mem::Ref<Image>& GetDepthStencilAttachmentTexture() { return _depthStencilAttachment; };
		const FramebufferSpec& GetSpecification() const { return _specs; }

		static ge::mem::Ref<Framebuffer> Create(const FramebufferSpec& spec);
	protected:
		GEVector<ge::mem::Ref<Image>> _colorAttachments;
		ge::mem::Ref<Image> _depthStencilAttachment;
		FramebufferSpec _specs;
	};
}