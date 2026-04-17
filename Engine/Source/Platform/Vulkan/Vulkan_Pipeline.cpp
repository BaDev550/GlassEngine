#include "gepch.h"
#include "Vulkan_Pipeline.h"
#include "Vulkan_Shader.h"

#include <glm/glm.hpp>

namespace ge::renderer {
	Vulkan_Pipeline::Vulkan_Pipeline(const PipelineSpecification& specs) {
		_specs = specs;
		Invalidate();
	}

	Vulkan_Pipeline::~Vulkan_Pipeline() {
		vkDestroyPipelineLayout(VK_RENDER_CONTEXT->GetDevice(), _layout, VK_ALLOCATOR_CALLBACKS);
		vkDestroyPipeline(VK_RENDER_CONTEXT->GetDevice(), _pipeline, VK_ALLOCATOR_CALLBACKS);
	}

	void Vulkan_Pipeline::Invalidate() {
		//PipelineConfig::Default(_config);
		//const auto& shader = _specs.shader.Cast<Vulkan_Shader>();
		//const auto& compiledData = shader->GetCompiledData();

		//VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		//vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		//vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(compiledData.AttribDescriptions.size());
		//vertexInputInfo.pVertexAttributeDescriptions = compiledData.AttribDescriptions.data();
		//vertexInputInfo.pVertexBindingDescriptions = &compiledData.BindingDescription;
		//vertexInputInfo.vertexBindingDescriptionCount = 1;

		//VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
		//vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		//vertexStageCreateInfo.module = shader->GetVertexModule();
		//vertexStageCreateInfo.pName = "main";

		//VkPipelineShaderStageCreateInfo fragStageCreateInfo{};
		//fragStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		//fragStageCreateInfo.module = shader->GetFragmentModule();
		//fragStageCreateInfo.pName = "main";
		//const VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = { vertexStageCreateInfo, fragStageCreateInfo };

		//std::vector<VkDescriptorSetLayout> layouts{};
		//for (auto& [set, layout] : shader->GetDescriptorLayouts())
		//	layouts.push_back(layout);

		//VkPipelineLayoutCreateInfo layoutCreateInfo{};
		//layoutCreateInfo.pSetLayouts = layouts.data();
		//layoutCreateInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		//layoutCreateInfo.pushConstantRangeCount = sizeof(glm::mat4);
		//vkCreatePipelineLayout(VK_RENDER_CONTEXT->GetDevice(), &layoutCreateInfo, VK_ALLOCATOR_CALLBACKS, &_layout);

		//VkGraphicsPipelineCreateInfo createInfo{};
		//createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		//createInfo.pNext = nullptr;
		//createInfo.stageCount = 2;
		//createInfo.pStages = shaderStageCreateInfos;
		//createInfo.pVertexInputState = &vertexInputInfo;
		//createInfo.pInputAssemblyState = &_config.inputAssembyCreateInfo;
		//createInfo.pViewportState = &_config.viewportStateCreateInfo;
		//createInfo.pRasterizationState = &_config.resterizationStateCreateInfo;
		//createInfo.pMultisampleState = &_config.multisampleStateCreateInfo;
		//createInfo.pDepthStencilState = &_config.depthStencilCreateInfo;
		//createInfo.pColorBlendState = &_config.colorBlendStateCreateInfo;
		//createInfo.pDynamicState = &_config.dynamicStateCreateInfo;
		//createInfo.layout = _layout;
		//vkCreateGraphicsPipelines(VK_RENDER_CONTEXT->GetDevice(), VK_NULL_HANDLE, 1, &createInfo, VK_ALLOCATOR_CALLBACKS, &_pipeline);
	}

	void PipelineConfig::Default(PipelineConfig& config)
	{
		config.inputAssembyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		config.inputAssembyCreateInfo.primitiveRestartEnable = VK_FALSE;

		config.viewportStateCreateInfo.viewportCount = 1;
		config.viewportStateCreateInfo.scissorCount = 1;
		config.viewportStateCreateInfo.pViewports = nullptr;
		config.viewportStateCreateInfo.pScissors = nullptr;

		config.resterizationStateCreateInfo = {};
		config.resterizationStateCreateInfo.depthClampEnable = VK_FALSE;
		config.resterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		config.resterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		config.resterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		config.resterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		config.resterizationStateCreateInfo.lineWidth = 1.0f;
		config.resterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
		config.resterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		config.resterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
		config.resterizationStateCreateInfo.depthBiasClamp = 0.0f;

		config.multisampleStateCreateInfo = {};
		config.multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
		config.multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		config.multisampleStateCreateInfo.minSampleShading = 1.0f;
		config.multisampleStateCreateInfo.pSampleMask = nullptr;
		config.multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
		config.multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

		config.colorBlendAttachment = {};
		config.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		config.colorBlendAttachment.blendEnable = VK_FALSE;
		config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		config.colorBlendStateCreateInfo = {};
		config.colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
		config.colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		config.colorBlendStateCreateInfo.attachmentCount = 1;
		config.colorBlendStateCreateInfo.pAttachments = &config.colorBlendAttachment;
		config.colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
		config.colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
		config.colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
		config.colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

		config.depthStencilCreateInfo = {};
		config.depthStencilCreateInfo.depthTestEnable = VK_TRUE;
		config.depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
		config.depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		config.depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
		config.depthStencilCreateInfo.minDepthBounds = 0.0f;
		config.depthStencilCreateInfo.maxDepthBounds = 1.0f;
		config.depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
		config.depthStencilCreateInfo.front = {};
		config.depthStencilCreateInfo.back = {};

		config.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		config.dynamicStateCreateInfo.pDynamicStates = config.dynamicStateEnables.data();
		config.dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(config.dynamicStateEnables.size());
	}
}