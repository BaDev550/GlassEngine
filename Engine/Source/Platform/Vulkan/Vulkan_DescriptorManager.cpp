#pragma once

#include "Vulkan_DescriptorManager.h"
#include "Vulkan_BindlessManager.h"
#include <ranges>

namespace ge::renderer {
	Vulkan_DescriptorManager::Vulkan_DescriptorManager(Vulkan_RenderContext& renderContext, const ShaderReflection& reflection)
	: _renderContext(renderContext) {

		// create dst set layout
		{
			GEVector<VkDescriptorSetLayoutBinding> bindings;
			for (auto &resource : reflection.resources | std::ranges::views::values) {
				bindings.emplace_back(
					resource.bindingIndex,
					utility::Vulkan_GetDescriptorType(resource.type),
					resource.resourceCount,
					// TODO: use more useful flag
					VK_SHADER_STAGE_ALL
				);
			}

			VkDescriptorSetLayoutCreateInfo dstLayoutCreateInfo{};
			dstLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			dstLayoutCreateInfo.bindingCount = bindings.size();
			dstLayoutCreateInfo.pBindings = bindings.data();
			vkCreateDescriptorSetLayout(_renderContext.GetDevice(), &dstLayoutCreateInfo, VK_ALLOCATOR_CALLBACKS, &_descriptorSetLayout);
		}

		// alloc dst set
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pSetLayouts = &_descriptorSetLayout;
			allocInfo.descriptorSetCount = 1;
			allocInfo.descriptorPool = _renderContext.GetGlobalDescriptorPool();

			vkAllocateDescriptorSets(_renderContext.GetDevice(), &allocInfo, &_descriptorSet);
		}

		// create pipeline layout
		{
			VkDescriptorSetLayout dstSetlayouts[4];
			dstSetlayouts[0] = _renderContext.GetBindlessManagersReadonlyImage().GetDstSetLayout();
			dstSetlayouts[1] = _renderContext.GetBindlessManagersWritableImage().GetDstSetLayout();
			dstSetlayouts[2] = _renderContext.GetBindlessManagersSampler().GetDstSetLayout();
			dstSetlayouts[3] = _descriptorSetLayout;

			VkPushConstantRange pushConstant{};
			pushConstant.offset = 0;
			pushConstant.size = 128;
			// TODO: use more useful flag
			pushConstant.stageFlags = VK_SHADER_STAGE_ALL;

			VkPipelineLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			createInfo.pSetLayouts = dstSetlayouts;
			createInfo.setLayoutCount = 4;
			createInfo.pushConstantRangeCount = 1;
			createInfo.pPushConstantRanges = &pushConstant;
			vkCreatePipelineLayout(_renderContext.GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &_pipelineLayout);
		}
	}

	void Vulkan_DescriptorManager::SetImage(
		const ShaderResource& resource, Vulkan_Image& image, ImageSubresource subresource, uint16_t resourceIndex) const noexcept {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = utility::Vulkan_OptimalImageLayout(image.GetDescRef().usageFlags);
		imageInfo.imageView = image.CreateGetImageView(subresource);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = 1;
		write.descriptorType = utility::Vulkan_GetDescriptorType(resource.type);
		write.dstArrayElement = resourceIndex;
		write.dstBinding = resource.bindingIndex;
		write.dstSet = _descriptorSet;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(_renderContext.GetDevice(), 1, &write, 0, nullptr);
	}

	void Vulkan_DescriptorManager::SetBuffer(
		const ShaderResource& resource, const Vulkan_Buffer& buffer, uint16_t firstElement, uint16_t elementCount, uint16_t resourceIndex) const noexcept {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer.GetVkBuffer();
		bufferInfo.offset = buffer.GetAlighnedElementSize() * firstElement;
		bufferInfo.range = buffer.GetAlighnedElementSize() * elementCount;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = 1;
		write.descriptorType = utility::Vulkan_GetDescriptorType(resource.type);
		write.dstArrayElement = resourceIndex;
		write.dstBinding = resource.bindingIndex;
		write.dstSet = _descriptorSet;
		write.pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(_renderContext.GetDevice(), 1, &write, 0, nullptr);
	}

	void Vulkan_DescriptorManager::SetSampler(
		const ShaderResource& resource, const Vulkan_Sampler& sampler, uint16_t resourceIndex) const noexcept {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = sampler.GetSampler();

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		write.dstArrayElement = resourceIndex;
		write.dstBinding = resource.bindingIndex;
		write.dstSet = _descriptorSet;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(_renderContext.GetDevice(), 1, &write, 0, nullptr);
	}
}