#pragma once

#include "GlassEngine/Renderer/Sampler.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_Sampler final : public Sampler {
	public:
		Vulkan_Sampler(const SamplerSpec& desc);
		virtual ~Vulkan_Sampler();
	private:
		VkSampler _sampler;
	};
}