#pragma once
#include "Vulkan_RenderContext.h"
#include "Vulkan_Image.h"
#include "Vulkan_Buffer.h"
#include "Vulkan_Sampler.h"

namespace ge::renderer {
	struct BindlessManagerSpec {
		ShaderResourceType resourceType;
	};

	class Vulkan_BindlessManager {
	public:
		Vulkan_BindlessManager(Vulkan_RenderContext& renderContext, const BindlessManagerSpec &spec);
		~Vulkan_BindlessManager();

		[[nodiscard]] uint32_t AddWritableImage(Vulkan_Image &image, ImageSubresource subresource);
		[[nodiscard]] uint32_t AddReadonlyImage(Vulkan_Image &image, ImageSubresource subresource);
		[[nodiscard]] uint32_t AddSampler(const Vulkan_Sampler &sampler);

		// any type
		void DeleteDescriptor(uint32_t index);

		[[nodiscard]] auto GetDstPool() const noexcept { return _descriptorPool; }
		[[nodiscard]] auto GetDstSetLayout() const noexcept { return _descriptorSetLayout; }
		[[nodiscard]] auto GetDstSet() const noexcept { return _descriptorSet; }
	private:
		uint32_t WriteDescriptor(const VkDescriptorImageInfo *imageInfo) noexcept;
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

	inline Vulkan_BindlessManager::~Vulkan_BindlessManager() {
		vkDestroyDescriptorSetLayout(_renderContext.GetDevice(), _descriptorSetLayout, VK_ALLOCATOR_CALLBACKS);
		vkDestroyDescriptorPool(_renderContext.GetDevice(), _descriptorPool, VK_ALLOCATOR_CALLBACKS);
	}
}