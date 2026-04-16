#include "gepch.h"
#include "Vulkan_RenderPass.h"

namespace ge::renderer {
	Vulkan_RenderPass::Vulkan_RenderPass(const ge::mem::Ref<Pipeline>& pipeline)
	{
	}

	void Vulkan_RenderPass::Begin(uint32_t layer)
	{
	}

	void Vulkan_RenderPass::End(uint32_t layer)
	{
	}

	void Vulkan_RenderPass::SetInput(std::string_view name, const ge::mem::Ref<Buffer>& buffer) {

	}

	void SetInput(std::string_view name, const ge::mem::Ref<Sampler>& sampler) {

	}

	void SetInput(std::string_view name, const ge::mem::Ref<Texture2D>& texture) {

	}
}