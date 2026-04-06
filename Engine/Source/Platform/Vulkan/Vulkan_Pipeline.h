#pragma once
#include "GlassEngine/Renderer/Pipeline.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	struct PipelineConfig {
		PipelineConfig() {}
		PipelineConfig(const PipelineConfig&) = delete;
		PipelineConfig& operator=(PipelineConfig&) = delete;

		VkPipelineInputAssemblyStateCreateInfo inputAssembyCreateInfo{};
		VkPipelineViewportStateCreateInfo      viewportStateCreateInfo{};
		VkPipelineRasterizationStateCreateInfo resterizationStateCreateInfo{};
		VkPipelineMultisampleStateCreateInfo   multisampleStateCreateInfo{};
		VkPipelineColorBlendAttachmentState    colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo    colorBlendStateCreateInfo{};
		VkPipelineDepthStencilStateCreateInfo  depthStencilCreateInfo{};

		GEVector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};

		static void Default(PipelineConfig& config);
	};

	class Vulkan_Pipeline : public Pipeline { // TODO (badev): look into this class again!
	public:
		Vulkan_Pipeline(const PipelineSpecification& specs);
		~Vulkan_Pipeline();

		void Invalidate();
		VkPipeline GetVKPipeline() const { return _pipeline; }
		VkPipelineLayout GetLayout() const { return _layout; }
	private:
		VkPipeline _pipeline;
		PipelineConfig _config;
		VkPipelineLayout _layout;
	};
}