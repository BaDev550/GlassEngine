#pragma once

#include "GlassEngine/Renderer/Pipeline.h"
#include "Vulkan_RenderContext.h"
#include "Vulkan_DescriptorManager.h"

namespace ge::renderer {
	class Vulkan_Pipeline : public Pipeline { // TODO (badev): look into this class again!
	public:
		Vulkan_Pipeline(const PipelineSpec& specs);
		~Vulkan_Pipeline();

		void Invalidate();
		[[nodiscard]] VkPipeline GetPipeline() const noexcept { return _pipeline; }
	private:
		VkPipeline _pipeline;
		Vulkan_DescriptorManager& _descriptorManager;
	};
}