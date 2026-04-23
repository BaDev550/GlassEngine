#include "gepch.h"
#include "Vulkan_BindlessManager.h"

namespace ge::renderer {
	[[nodiscard]] static constexpr uint32_t GetDescriptorCount(ShaderResourceType type) noexcept {
		switch (type) {
		case ShaderResourceType::UniformBuffer: GE_ASSERT(ShaderResourceType::UniformBuffer != type, "Vulkan_BindlessManager not support UniformBuffer");
		case ShaderResourceType::ReadonlyImage: return 200'000;
		case ShaderResourceType::WritableImage: return 10'000;
		case ShaderResourceType::Sampler: return 1'000; // this is enough, reuse
		case ShaderResourceType::ReadonlyBuffer:
			GE_ASSERT(ShaderResourceType::ReadonlyBuffer != type, "Vulkan_BindlessManager not support ReadonlyBuffer, use bda"); return 0;
		case ShaderResourceType::WritableBuffer:
			GE_ASSERT(ShaderResourceType::WritableBuffer != type, "Vulkan_BindlessManager not support WritableBuffer, use bda"); return 0;
		}
	}

	Vulkan_BindlessManager::Vulkan_BindlessManager(Vulkan_RenderContext& renderContext, const BindlessManagerSpec& spec)
		: _resourceType(spec.resourceType), _renderContext(renderContext), _descriptorCount(GetDescriptorCount(spec.resourceType)) {
		// create dst pool
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = utility::Vulkan_GetDescriptorType(_resourceType);
			poolSize.descriptorCount = _descriptorCount;

			VkDescriptorPoolCreateInfo dstPoolCreateInfo{};
			dstPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			dstPoolCreateInfo.maxSets = 1;
			dstPoolCreateInfo.poolSizeCount = 1;
			dstPoolCreateInfo.pPoolSizes = &poolSize;
			dstPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
			vkCreateDescriptorPool(_renderContext.GetDevice(), &dstPoolCreateInfo, VK_ALLOCATOR_CALLBACKS, &_descriptorPool);
		}

		// create dst set layout
		{
			const VkDescriptorBindingFlags bindingFlags
				= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
				| VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
				| VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT
				// pre turing gpu's don't have update after bind for uniform buffers
				| VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
				;

			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo{};
			bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			bindingFlagsCreateInfo.bindingCount = 1;
			bindingFlagsCreateInfo.pBindingFlags = &bindingFlags;

			VkDescriptorSetLayoutBinding binding{};
			binding.binding = 0;
			binding.descriptorCount = _descriptorCount;
			binding.descriptorType = utility::Vulkan_GetDescriptorType(_resourceType);
			binding.stageFlags = VK_SHADER_STAGE_ALL;

			VkDescriptorSetLayoutCreateInfo dstLayoutCreateInfo{};
			dstLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			dstLayoutCreateInfo.pBindings = &binding;
			dstLayoutCreateInfo.bindingCount = 1;
			// pre turing gpu's don't have update after bind for uniform buffers
			dstLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
			dstLayoutCreateInfo.pNext = &bindingFlagsCreateInfo;
			vkCreateDescriptorSetLayout(_renderContext.GetDevice(), &dstLayoutCreateInfo, VK_ALLOCATOR_CALLBACKS, &_descriptorSetLayout);
		}

		// alloc dst set
		{
			VkDescriptorSetVariableDescriptorCountAllocateInfo varCountInfo{};
			varCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			varCountInfo.descriptorSetCount = 1;
			const uint32_t count = _descriptorCount;
			varCountInfo.pDescriptorCounts = &count;

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pSetLayouts = &_descriptorSetLayout;
			allocInfo.descriptorSetCount = 1;
			allocInfo.descriptorPool = _descriptorPool;
			allocInfo.pNext = &varCountInfo;

			vkAllocateDescriptorSets(_renderContext.GetDevice(), &allocInfo, &_descriptorSet);
		}
	}

	uint32_t Vulkan_BindlessManager::AddReadonlyImage(Vulkan_Image &image, ImageSubresource subresource) {
		// TODO (dnm): write error
		GE_ASSERT(ShaderResourceType::ReadonlyImage == _resourceType, "");
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = utility::Vulkan_OptimalImageLayout(image.GetDescRef().usageFlags);
		imageInfo.imageView = image.CreateGetImageView(subresource);
		return WriteDescriptor(&imageInfo);
	}

	uint32_t Vulkan_BindlessManager::AddWritableImage(Vulkan_Image &image, ImageSubresource subresource) {
		// TODO (dnm): write error
		GE_ASSERT(ShaderResourceType::WritableImage == _resourceType, "");
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = utility::Vulkan_OptimalImageLayout(image.GetDescRef().usageFlags);
		imageInfo.imageView = image.CreateGetImageView(subresource);
		return WriteDescriptor(&imageInfo);
	}

	uint32_t Vulkan_BindlessManager::AddSampler(const Vulkan_Sampler& sampler) {
		GE_ASSERT(ShaderResourceType::WritableImage == _resourceType, "");
		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = sampler.GetSampler();
		return WriteDescriptor(&imageInfo);
	}

	void Vulkan_BindlessManager::DeleteDescriptor(uint32_t index) {
		GE_ASSERT(index < _descriptorCount, "index out of bounds");
		_deletedIndex.push_back(index);
	}

	uint32_t Vulkan_BindlessManager::WriteDescriptor(const VkDescriptorImageInfo* imageInfo) noexcept {
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = 1;
		write.descriptorType = utility::Vulkan_GetDescriptorType(_resourceType);
		write.dstArrayElement = GetIndex();
		write.dstBinding = 0;
		write.dstSet = _descriptorSet;
		write.pBufferInfo = nullptr;
		write.pImageInfo = imageInfo;
		vkUpdateDescriptorSets(_renderContext.GetDevice(), 1, &write, 0, nullptr);
		return write.dstArrayElement;
	}

	uint64_t Vulkan_BindlessManager::GetIndex() noexcept {
		if (_descriptorCount != _usage) {
			return _usage++;
		}
		const auto index = _deletedIndex.back();
		_deletedIndex.pop_back();
		return index;
	}
}
