#pragma once
#include "GlassEngine/Core/Core.h"
#include <initializer_list>
#include "Types.h"

namespace ge::renderer {
	struct FramebufferAttachmentSpecification {
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<ImageFormat>& attachments) : Attachments(attachments) {}
		GEVector<ImageFormat> Attachments;
	};
	
	struct FramebufferSpecification {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t layers = 0;
		float depthClearValue = 1.0f;
		glm::vec3 clearColor{ 0.0f };
		FramebufferAttachmentSpecification Attachments;
		bool IsSwapchain = false;
	};

	class Framebuffer : public ge::mem::RefCounted {
	public:
		virtual void Invalidate(const FramebufferSpecification& spec) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		uint32_t GetWidth() const { return _specs.width; }
		uint32_t GetHeight() const { return _specs.height; }
		uint32_t GetAttachmentCount() const { return static_cast<uint32_t>(_specs.Attachments.Attachments.size()); }
		GEVector<ImageFormat>& GetAttachments() { return _specs.Attachments.Attachments; }
		ge::mem::Ref<Image>& GetColorAttachmentTexture(uint32_t index = 0) { return _colorAttachments[index]; };
		ge::mem::Ref<Image>& GetDepthAttachmentTexture(uint32_t index = 0) { return _depthAttachments[index]; };
		const FramebufferSpecification& GetSpecification() const { return _specs; }

		static ge::mem::Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	protected:
		GEVector<ge::mem::Ref<Image>> _colorAttachments;
		GEVector<ge::mem::Ref<Image>> _depthAttachments;
		FramebufferSpecification _specs;
	};
}