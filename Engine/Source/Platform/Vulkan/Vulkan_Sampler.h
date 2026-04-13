#pragma once

#include "GlassEngine/Renderer/Sampler.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_Sampler final : public Sampler {
	public:
		Vulkan_Sampler(const SamplerSpec& desc);
		~Vulkan_Sampler();
	private:
		VkSampler _sampler;
	};

	inline Vulkan_Sampler::~Vulkan_Sampler() {
		vkDestroySampler(VK_RENDER_CONTEXT->GetDevice(), _sampler, VK_ALLOCATOR_CALLBACKS);
	}
}