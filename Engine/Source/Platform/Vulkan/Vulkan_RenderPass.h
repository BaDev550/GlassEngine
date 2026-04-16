#pragma once
#include "GlassEngine/Renderer/RenderPass.h"
#include "Vulkan_DescriptorManager.h"

namespace ge::renderer {
	class Vulkan_RenderPass : public RenderPass {
	public:
		Vulkan_RenderPass(const ge::mem::Ref<Pipeline>& pipeline);
		virtual void Begin(uint32_t layer = 0) override;
		virtual void End(uint32_t layer = 0) override;

		virtual void SetInput(std::string_view name, const ge::mem::Ref<Buffer>& buffer) override;
		virtual void SetInput(std::string_view name, const ge::mem::Ref<Sampler>& sampler) override;
		virtual void SetInput(std::string_view name, const ge::mem::Ref<Texture2D>& texture) override;
	private:
		ge::mem::Ref<Pipeline> _pipeline;
		ge::mem::Scope<Vulkan_DescriptorManager> _descriptorManager;
	};
}