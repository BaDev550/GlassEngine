#pragma once
#include "Vulkan_RenderContext.h"
#include "Vulkan_Image.h"
#include "Vulkan_Buffer.h"

namespace ge::renderer {
	struct BindlessManagerSpec {
		ShaderResourceType resourceType;
	};

	class BindlessManager {
	public:
		BindlessManager(Vulkan_RenderContext& renderContext, const BindlessManagerSpec &spec);

		uint32_t AddWritableImage(Vulkan_Image* image, ImageSubresource subresource);
		uint32_t AddReadonlyImage(Vulkan_Image* image, ImageSubresource subresource);
		uint32_t AddUnifromBuffer(Vulkan_Buffer* buffer, uint16_t firstElement, uint16_t elementCount);
		// TODO (0x): sampler 
		uint32_t AddSampler();

		// any type
		void DeleteDescriptor(uint32_t index);
	private:
		uint32_t WriteDescriptor(const VkDescriptorImageInfo *imageInfo, const VkDescriptorBufferInfo* bufferInfo) noexcept;
		uint64_t GetIndex() noexcept;
		GEVector<uint32_t> _deletedIndex;

		VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
		VkDescriptorSet _descriptorSet{ VK_NULL_HANDLE };
		VkDescriptorSetLayout _descriptorSetLayout{ VK_NULL_HANDLE };
		 
		uint32_t _usage{ 0 };
		const uint32_t _descriptorCount;
		const ShaderResourceType _resourceType;

		Vulkan_RenderContext& _renderContext;
	};
}