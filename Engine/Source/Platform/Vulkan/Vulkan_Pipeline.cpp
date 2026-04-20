#include "gepch.h"
#include "Vulkan_Pipeline.h"
#include "Vulkan_Shader.h"

#include <glm/glm.hpp>
#include <GlassEngine/Utilities/Counter.h>

namespace ge::renderer {
	Vulkan_Pipeline::Vulkan_Pipeline(const PipelineSpec& specs)
		: Pipeline(specs), _descriptorManager(*(new Vulkan_DescriptorManager(*VK_RENDER_CONTEXT, specs.shader->GetReflection()))) {
		_specs = specs;
		Invalidate();
	}

	Vulkan_Pipeline::~Vulkan_Pipeline() {
		vkDestroyPipeline(VK_RENDER_CONTEXT->GetDevice(), _pipeline, VK_ALLOCATOR_CALLBACKS);
	}

	void Vulkan_Pipeline::Invalidate() {
		const auto& shader = _specs.shader.Cast<Vulkan_Shader>();

		GEVector<VkVertexInputAttributeDescription> vertexAttribDescs{};
		for (const auto vertAttrib : _specs.inputAssemblySpec.vertexAttributes) {
			vertexAttribDescs.emplace_back(
				static_cast<uint32_t>(vertAttrib.location),
				static_cast<uint32_t>(vertAttrib.binding),
				utility::Vulkan_GetVertexFormat(vertAttrib.format),
				static_cast<uint32_t>(vertAttrib.offset)
			);
		}

		GEVector<VkVertexInputBindingDescription> vertexBindingDescs{};
		for (const auto vertBinding : _specs.inputAssemblySpec.vertexBindings) {
			vertexBindingDescs.emplace_back(
				static_cast<uint32_t>(vertBinding.binding),
				static_cast<uint32_t>(vertBinding.stride),
				utility::Vulkan_GetVertexInputRate(vertBinding.inputRate)
			);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = vertexAttribDescs.size();
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttribDescs.data();
		vertexInputInfo.vertexBindingDescriptionCount = vertexBindingDescs.size();
		vertexInputInfo.pVertexBindingDescriptions = vertexBindingDescs.data();

		VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
		vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexStageCreateInfo.module = shader->GetShaderModule();
		vertexStageCreateInfo.pName = "VertMain";

		VkPipelineShaderStageCreateInfo fragStageCreateInfo{};
		fragStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStageCreateInfo.module = shader->GetShaderModule();
		fragStageCreateInfo.pName = "FragMain";
		const VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = { vertexStageCreateInfo, fragStageCreateInfo };

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		inputAssemblyState.topology = utility::Vulkan_GetPrimitiveTopology(_specs.inputAssemblySpec.topology);

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.scissorCount = 1;
		viewportState.viewportCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.polygonMode = utility::Vulkan_GetPolygonMode(_specs.resterizerSpec.polygonMode);
		rasterizationState.cullMode = utility::Vulkan_GetCullMode(_specs.resterizerSpec.cullMode);
		rasterizationState.lineWidth = 1.f;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.lineWidth = 1.0f;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.depthBiasConstantFactor = 0.0f;
		rasterizationState.depthBiasSlopeFactor = 0.0f;
		rasterizationState.depthBiasClamp = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_RENDER_CONTEXT->GetSampleCount(_specs.resterizerSpec.sampleCount);

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;
		depthStencilState.depthTestEnable = _specs.depthStencilSpec.depthTestEnable;
		depthStencilState.depthWriteEnable = _specs.depthStencilSpec.depthWriteEnable;
		depthStencilState.stencilTestEnable = _specs.depthStencilSpec.stencilTestEnable;
		depthStencilState.depthCompareOp = utility::Vulkan_GetCompareOp(_specs.depthStencilSpec.depthTestCompareOp);
		depthStencilState.front.compareOp = utility::Vulkan_GetCompareOp(_specs.depthStencilSpec.stencilFrontOp.compareOp);
		depthStencilState.front.compareMask = _specs.depthStencilSpec.stencilCompareMask;
		depthStencilState.front.writeMask = _specs.depthStencilSpec.stencilWriteMask;
		depthStencilState.front.depthFailOp = utility::Vulkan_GetStencilOp(_specs.depthStencilSpec.stencilFrontOp.depthFailOp);
		depthStencilState.front.failOp = utility::Vulkan_GetStencilOp(_specs.depthStencilSpec.stencilFrontOp.failOp);
		depthStencilState.front.passOp = utility::Vulkan_GetStencilOp(_specs.depthStencilSpec.stencilFrontOp.passOp);
		depthStencilState.back.compareOp = utility::Vulkan_GetCompareOp(_specs.depthStencilSpec.stencilBackOp.compareOp);
		depthStencilState.back.compareMask = _specs.depthStencilSpec.stencilCompareMask;
		depthStencilState.back.writeMask = _specs.depthStencilSpec.stencilWriteMask;
		depthStencilState.back.depthFailOp = utility::Vulkan_GetStencilOp(_specs.depthStencilSpec.stencilBackOp.depthFailOp);
		depthStencilState.back.failOp = utility::Vulkan_GetStencilOp(_specs.depthStencilSpec.stencilBackOp.failOp);
		depthStencilState.back.passOp = utility::Vulkan_GetStencilOp(_specs.depthStencilSpec.stencilBackOp.passOp);

		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates{};
		for ([[maybe_unused]] auto _ : Counter(_specs.targetFramebuffer->GetAttachmentCount())) {
			colorBlendAttachmentStates.emplace_back(
				_specs.blendSpec.blendEnabled,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			);
		}

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = colorBlendAttachmentStates.size();
		colorBlendState.pAttachments = colorBlendAttachmentStates.data();

		GEVector dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		if (_specs.depthStencilSpec.stencilTestEnable)
			dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStates.data();
		dynamicState.dynamicStateCount = dynamicStates.size();

		GEVector<VkFormat> colorAttachmentFormats;
		for (const auto format : _specs.targetFramebuffer->GetAttachments()) {
			colorAttachmentFormats.push_back(utility::Vulkan_GetImageFormat(format));
		}

		VkPipelineRenderingCreateInfo renderingInfo{};
		renderingInfo.colorAttachmentCount = colorAttachmentFormats.size();
		renderingInfo.pColorAttachmentFormats = colorAttachmentFormats.data();
		//renderingInfo.depthAttachmentFormat = 0;
		//renderingInfo.stencilAttachmentFormat = 0;

		VkGraphicsPipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = &renderingInfo;
		createInfo.stageCount = 2;
		createInfo.pStages = shaderStageCreateInfos;
		createInfo.pVertexInputState = &vertexInputInfo;
		createInfo.pInputAssemblyState = &inputAssemblyState;
		createInfo.pViewportState = &viewportState;
		createInfo.pRasterizationState = &rasterizationState;
		createInfo.pMultisampleState = &multisampleState;
		createInfo.pDepthStencilState = &depthStencilState;
		createInfo.pColorBlendState = &colorBlendState;
		createInfo.pDynamicState = &dynamicState;
		createInfo.layout = _descriptorManager.GetPipelineLayout();
		vkCreateGraphicsPipelines(VK_RENDER_CONTEXT->GetDevice(), VK_NULL_HANDLE, 1, &createInfo, VK_ALLOCATOR_CALLBACKS, &_pipeline);
	}
}