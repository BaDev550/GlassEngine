#include "Vulkan_DescriptorManager.h"
#include "GlassEngine/Core/String.h"
#include "GlassEngine/Utilities/Logger.h"
#include "Platform/Vulkan/Vulkan_RenderContext.h"
#include "Platform/Vulkan/Vulkan_Types.h"
#include <array>
#include <cstdint>
#include <ranges>
#include <vulkan/vulkan_core.h>

namespace ge::renderer {
	Vulkan_DescriptorManagerDefault::Vulkan_DescriptorManagerDefault(Vulkan_RenderContext& renderContext)
	: Vulkan_DescriptorManager(renderContext) {
		CreateBindlessDescriptorPoolAndSet(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 
			VULKAN_SAMPLED_IMAGE_COUNT, _renderContext.GetDeviceFeatures().partiallyBoundForSampledImage,
			"Bindless Readonly Image", _readonlyImagePool, _readonlyImageSetLayout, _readonlyImageSet);

		CreateBindlessDescriptorPoolAndSet(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 
			VULKAN_STORAGE_IMAGE_COUNT, _renderContext.GetDeviceFeatures().partiallyBoundForStorageImage,
			"Bindless Writable Image", _writableImagePool, _writableImageSetLayout, _writableImageSet);

		CreateBindlessDescriptorPoolAndSet(VK_DESCRIPTOR_TYPE_SAMPLER, 
			VULKAN_SAMPLER_COUNT, _renderContext.GetDeviceFeatures().partiallyBoundForSampler,
			"Bindless Sampler", _samplerPool, _samplerSetLayout, _samplerSet);

		CreateGlobalDescriptorPoolAndSet();
		CreatePipelineLayout();
	}

	void Vulkan_DescriptorManagerDefault::CreatePipelineLayout() {
		{
			VkPushConstantRange pushConstant{};
			pushConstant.offset = 0;
			// anv, radv, and offical nvidia driver support 256byte push constant
			pushConstant.size = 256;
			pushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

			const VkDescriptorSetLayout setLayouts[4]{
				_globalDescriptorSetLayout,
				_readonlyImageSetLayout,
				_writableImageSetLayout,
				_samplerSetLayout,
			};

			VkPipelineLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			createInfo.pSetLayouts = setLayouts;
			createInfo.setLayoutCount = 4;
			createInfo.pushConstantRangeCount = 1;
			createInfo.pPushConstantRanges = &pushConstant;
			vkCreatePipelineLayout(_renderContext.GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &_pipelineLayout);

			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectHandle = reinterpret_cast<uint64_t>(_pipelineLayout);
			nameInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
			nameInfo.pObjectName = "Global Pipeline Layout";

			// TODO (dnm): fix vkSetDebugUtilsObjectNameEXT(): Returned error VK_ERROR_OUT_OF_HOST_MEMORY. error
			// vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
		}
	}

	void Vulkan_DescriptorManagerDefault::CreateGlobalDescriptorPoolAndSet() {
		{
			GEVector<VkDescriptorPoolSize> poolSizes{};
			poolSizes.emplace_back(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				10
			);

			VkDescriptorPoolCreateInfo dstPoolCreateInfo{};
			dstPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			dstPoolCreateInfo.maxSets = 1;
			dstPoolCreateInfo.poolSizeCount = poolSizes.size();
			dstPoolCreateInfo.pPoolSizes = poolSizes.data();
			dstPoolCreateInfo.flags = VkDescriptorPoolCreateFlags{};
			vkCreateDescriptorPool(VK_RENDER_CONTEXT->GetDevice(), &dstPoolCreateInfo, VK_ALLOCATOR_CALLBACKS, &_globalDescriptorPool);
		
			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectHandle = reinterpret_cast<uint64_t>(_globalDescriptorPool);
			nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
			nameInfo.pObjectName = "Global Descriptor Pool";

			// TODO (dnm): fix vkSetDebugUtilsObjectNameEXT(): Returned error VK_ERROR_OUT_OF_HOST_MEMORY. error 
			// vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
		}

		{
			GEVector<VkDescriptorSetLayoutBinding> bindings{};
			bindings.emplace_back(
				0,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				1,
				VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT
			);

			VkDescriptorSetLayoutCreateInfo dstLayoutCreateInfo{};
			dstLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			dstLayoutCreateInfo.bindingCount = bindings.size();
			dstLayoutCreateInfo.pBindings = bindings.data();
			vkCreateDescriptorSetLayout(VK_RENDER_CONTEXT->GetDevice(), &dstLayoutCreateInfo, VK_ALLOCATOR_CALLBACKS, &_globalDescriptorSetLayout);
		
			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectHandle = reinterpret_cast<uint64_t>(_globalDescriptorSetLayout);
			nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
			nameInfo.pObjectName = "Global Descriptor Set Layout";

			// TODO (dnm): fix vkSetDebugUtilsObjectNameEXT(): Returned error VK_ERROR_OUT_OF_HOST_MEMORY. error 
			// vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
		}

		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pSetLayouts = &_globalDescriptorSetLayout;
			allocInfo.descriptorSetCount = 1;
			allocInfo.descriptorPool = _globalDescriptorPool;

			vkAllocateDescriptorSets(VK_RENDER_CONTEXT->GetDevice(), &allocInfo, &_globalDescriptorSet);

			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectHandle = reinterpret_cast<uint64_t>(_globalDescriptorSet);
			nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
			nameInfo.pObjectName = "Global Descriptor Set";

			// TODO (dnm): fix vkSetDebugUtilsObjectNameEXT(): Returned error VK_ERROR_OUT_OF_HOST_MEMORY. error 
			// vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
		}
	}

	void Vulkan_DescriptorManagerDefault::CreateBindlessDescriptorPoolAndSet(
			VkDescriptorType type, uint32_t descriptorCount, bool partiallyBound, std::string_view debugName,
			VkDescriptorPool& outPool, VkDescriptorSetLayout& outSetLayout, VkDescriptorSet& outSet) {
		// create dst pool
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = type;
			poolSize.descriptorCount = descriptorCount;

			VkDescriptorPoolCreateInfo dstPoolCreateInfo{};
			dstPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			dstPoolCreateInfo.maxSets = 1;
			dstPoolCreateInfo.poolSizeCount = 1;
			dstPoolCreateInfo.pPoolSizes = &poolSize;
			dstPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT * partiallyBound;
			vkCreateDescriptorPool(_renderContext.GetDevice(), &dstPoolCreateInfo, VK_ALLOCATOR_CALLBACKS, &outPool);

			const auto name = GEString(debugName) + " Descriptor Pool";

			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectHandle = reinterpret_cast<uint64_t>(outPool);
			nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
			nameInfo.pObjectName = name.c_str();

			// TODO (dnm): fix vkSetDebugUtilsObjectNameEXT(): Returned error VK_ERROR_OUT_OF_HOST_MEMORY. error 
			// vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
		}

		// create dst set layout
		{
			const VkDescriptorBindingFlags bindingFlags
				= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
				| VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
				| VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT
				// pre turing gpu's don't have update after bind for uniform buffers
				| (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT * partiallyBound	)
				;

			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo{};
			bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			bindingFlagsCreateInfo.bindingCount = 1;
			bindingFlagsCreateInfo.pBindingFlags = &bindingFlags;

			VkDescriptorSetLayoutBinding binding{};
			binding.binding = 0;
			binding.descriptorCount = descriptorCount;
			binding.descriptorType = type;
			binding.stageFlags = VK_SHADER_STAGE_ALL;

			VkDescriptorSetLayoutCreateInfo dstLayoutCreateInfo{};
			dstLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			dstLayoutCreateInfo.pBindings = &binding;
			dstLayoutCreateInfo.bindingCount = 1;
			// pre turing gpu's don't have update after bind for uniform buffers
			dstLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT * partiallyBound;
			dstLayoutCreateInfo.pNext = &bindingFlagsCreateInfo;
			vkCreateDescriptorSetLayout(_renderContext.GetDevice(), &dstLayoutCreateInfo, VK_ALLOCATOR_CALLBACKS, &outSetLayout);
		
			const auto name = GEString(debugName) + " Descriptor Set Layout";

			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectHandle = reinterpret_cast<uint64_t>(outSetLayout);
			nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
			nameInfo.pObjectName = name.c_str();

			// TODO (dnm): fix vkSetDebugUtilsObjectNameEXT(): Returned error VK_ERROR_OUT_OF_HOST_MEMORY. error 
			// vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
		}

		// alloc dst set
		{
			VkDescriptorSetVariableDescriptorCountAllocateInfo varCountInfo{};
			varCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			varCountInfo.descriptorSetCount = 1;
			const uint32_t count = descriptorCount;
			varCountInfo.pDescriptorCounts = &count;

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pSetLayouts = &outSetLayout;
			allocInfo.descriptorSetCount = 1;
			allocInfo.descriptorPool = outPool;
			allocInfo.pNext = &varCountInfo;

			vkAllocateDescriptorSets(_renderContext.GetDevice(), &allocInfo, &outSet);

			const auto name = GEString(debugName) + " Descriptor Set";

			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectHandle = reinterpret_cast<uint64_t>(outSet);
			nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
			nameInfo.pObjectName = name.c_str();

			// TODO (dnm): fix vkSetDebugUtilsObjectNameEXT(): Returned error VK_ERROR_OUT_OF_HOST_MEMORY. error 
			// vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
		}
	}

	uint32_t Vulkan_DescriptorManagerDefault::RegisterReadonlyImage(Vulkan_Image &image, ImageSubresource subresource) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = utility::Vulkan_OptimalImageLayout(image.GetSpecRef().usageFlags);
		imageInfo.imageView = image.CreateGetImageView(subresource);

		uint32_t index{};
		if (_readonlyImageUsageCount + 1 < VULKAN_SAMPLED_IMAGE_COUNT) {
			index = _readonlyImageUsageCount++;
		}
		else if (!_readonlyImageDeletedIndices.empty()) {
			index = _readonlyImageDeletedIndices.back();
			_readonlyImageDeletedIndices.pop_back();
		}
		else {
			GE_GRAPHICS_ERROR("Bindless readonly image limit exceeded!");
		}

		WriteDescriptorForBindless(_readonlyImageSet, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, &imageInfo, index);
		return index;
	}

	uint32_t Vulkan_DescriptorManagerDefault::RegisterWritableImage(Vulkan_Image &image, ImageSubresource subresource) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView = image.CreateGetImageView(subresource);

		uint32_t index{};
		if (_writableImageUsageCount + 1 < VULKAN_STORAGE_IMAGE_COUNT) {
			index = _writableImageUsageCount++;
		}
		else if (!_writableImageDeletedIndices.empty()) {
			index = _writableImageDeletedIndices.back();
			_writableImageDeletedIndices.pop_back();
		}
		else {
			GE_GRAPHICS_ERROR("Bindless writable image limit exceeded!");
		}

		WriteDescriptorForBindless(_writableImageSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &imageInfo, index);
		return index;
	}

	uint32_t Vulkan_DescriptorManagerDefault::RegisterSampler(const Vulkan_Sampler &sampler) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = sampler.GetSampler();

		uint32_t index{};
		if (_samplerUsageCount + 1 < VULKAN_SAMPLER_COUNT) {
			index = _samplerUsageCount++;
		}
		else if (!_samplerDeletedIndices.empty()) {
			index = _samplerDeletedIndices.back();
			_samplerDeletedIndices.pop_back();
		}
		else {
			GE_GRAPHICS_ERROR("Bindless sampler limit exceeded!");
		}

		WriteDescriptorForBindless(_samplerSet, VK_DESCRIPTOR_TYPE_SAMPLER, &imageInfo, index);
		return index;
	}

	void Vulkan_DescriptorManagerDefault::WriteDescriptorForBindless(VkDescriptorSet set, VkDescriptorType type, const VkDescriptorImageInfo *imageInfo, uint32_t index) noexcept{
		VkWriteDescriptorSet writeSet{};
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.dstSet = set;
		writeSet.dstBinding = 0;
		writeSet.dstArrayElement = index;
		writeSet.descriptorCount = 1;
		writeSet.descriptorType = type;
		writeSet.pImageInfo = imageInfo;

		vkUpdateDescriptorSets(VK_RENDER_CONTEXT->GetDevice(), 1, &writeSet, 0, nullptr);
	}

	void Vulkan_DescriptorManagerDefault::UnregisterReadonlyImage(uint32_t handle) {
		_readonlyImageDeletedIndices.push_back(handle);
	}

	void Vulkan_DescriptorManagerDefault::UnregisterWritableImage(uint32_t handle) {
		_writableImageDeletedIndices.push_back(handle);
	}

	void Vulkan_DescriptorManagerDefault::UnregisterSampler(uint32_t handle) {
		_samplerDeletedIndices.push_back(handle);
	}

	void Vulkan_DescriptorManagerDefault::PushConstant(VkCommandBuffer cmd, const void *ptr, uint16_t size, uint16_t offset) {
		vkCmdPushConstants(cmd, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, offset, size, ptr);
	}

	void Vulkan_DescriptorManagerDefault::BindDescriptors(VkCommandBuffer cmd) {
		const VkDescriptorSet sets[4]{
			_globalDescriptorSet,
			_readonlyImageSet,
			_writableImageSet,
			_samplerSet,
		};

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 4, sets, 0, nullptr);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 4, sets, 0, nullptr);
		std::array<uint8_t, 256> emptyConstant{};
		PushConstant(cmd, emptyConstant.data(), 256, 0);
	}

	void Vulkan_DescriptorManagerDefault::SetUniformBuffer(const Vulkan_Buffer& buffer, uint32_t binding) {
		VkDescriptorBufferInfo info{};
		info.buffer = buffer.GetVkBuffer();
		info.offset = 0;
		info.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet writeSet{};
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.dstSet = _globalDescriptorSet;
		writeSet.dstBinding = 0;
		writeSet.dstBinding = binding;
		writeSet.descriptorCount = 1;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeSet.pBufferInfo = &info;

		vkUpdateDescriptorSets(VK_RENDER_CONTEXT->GetDevice(), 1, &writeSet, 0, nullptr);
	}
}
