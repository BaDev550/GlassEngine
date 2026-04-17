#pragma once
#include "GlassEngine/Renderer/RenderPass.h"
#include "Vulkan_DescriptorManager.h"

namespace ge::renderer {
	class Vulkan_RenderPass : public RenderPass {
	public:
		Vulkan_RenderPass(const ge::mem::Ref<Pipeline>& pipeline);
		virtual void Begin(uint32_t layer = 0) override;
		virtual void End(uint32_t layer = 0) override;

		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Buffer>& buffer, uint16_t firstElement, uint16_t elementCount, uint16_t resourceIndex) override;
		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Sampler>& sampler, uint16_t resourceIndex) override;
		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Image>& texture, ImageSubresource subresource, uint16_t resourceIndex) override;
	private:
		ge::mem::Scope<Vulkan_DescriptorManager> _descriptorManager;
	};
}