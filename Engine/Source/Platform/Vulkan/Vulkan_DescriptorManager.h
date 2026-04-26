#pragma once

#include <GlassEngine/Renderer/ShaderCompiler.h>
#include <cstdint>
#include <string_view>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>
#include "Vulkan_Image.h"
#include "Vulkan_Buffer.h"
#include "Vulkan_Sampler.h"

namespace ge::renderer {
	class Vulkan_DescriptorManager {
	public:
		Vulkan_DescriptorManager(Vulkan_RenderContext& renderContext) : _renderContext(renderContext) {}
		virtual ~Vulkan_DescriptorManager() = default;
		[[nodiscard]] virtual uint32_t RegisterReadonlyImage(Vulkan_Image &image, ImageSubresource subresource) = 0;
		[[nodiscard]] virtual uint32_t RegisterWritableImage(Vulkan_Image &image, ImageSubresource subresource) = 0;
		[[nodiscard]] virtual uint32_t RegisterSampler(const Vulkan_Sampler &sampler) = 0;
		
		virtual void UnregisterReadonlyImage(uint32_t handle) = 0;
		virtual void UnregisterWritableImage(uint32_t handle) = 0;
		virtual void UnregisterSampler(uint32_t handle) = 0;

		virtual void PushConstant(VkCommandBuffer cmd, const void *ptr, uint16_t size, uint16_t offset) = 0;
		virtual void BindDescriptors(VkCommandBuffer cmd) = 0;
	protected:
		Vulkan_RenderContext& _renderContext;
	};

	class Vulkan_DescriptorManagerDefault final : public Vulkan_DescriptorManager {
	public:
		Vulkan_DescriptorManagerDefault(Vulkan_RenderContext& renderContext);
		~Vulkan_DescriptorManagerDefault();

		[[nodiscard]] virtual uint32_t RegisterReadonlyImage(Vulkan_Image &image, ImageSubresource subresource) override;
		[[nodiscard]] virtual uint32_t RegisterWritableImage(Vulkan_Image &image, ImageSubresource subresource) override;
		[[nodiscard]] virtual uint32_t RegisterSampler(const Vulkan_Sampler &sampler) override;

		virtual void UnregisterReadonlyImage(uint32_t handle) override;
		virtual void UnregisterWritableImage(uint32_t handle) override;
		virtual void UnregisterSampler(uint32_t handle) override;

		virtual void PushConstant(VkCommandBuffer cmd, const void *ptr, uint16_t size, uint16_t offset) override;
		virtual void BindDescriptors(VkCommandBuffer cmd) override;

		[[nodiscard]] auto GetGlobalDescriptorSet() const noexcept { return _globalDescriptorSet; }
		[[nodiscard]] auto GetReadonlyImageSet() const noexcept { return _readonlyImageSet; }
		[[nodiscard]] auto GetWritableImageSet() const noexcept { return _writableImageSet; }
		[[nodiscard]] auto GetSamplerSet() const noexcept { return _samplerSet; }

		[[nodiscard]] auto GetGlobalDescriptorSetLayout() const noexcept { return _globalDescriptorSetLayout; }
		[[nodiscard]] auto GetReadonlyImageSetLayout() const noexcept { return _readonlyImageSetLayout; }
		[[nodiscard]] auto GetWritableImageSetLayout() const noexcept { return _writableImageSetLayout; }
		[[nodiscard]] auto GetSamplerSetLayout() const noexcept { return _samplerSetLayout; }

		[[nodiscard]] auto GetGlobalDescriptorPool() const noexcept { return _globalDescriptorPool; }
		[[nodiscard]] auto GetReadonlyImagePool() const noexcept { return _readonlyImagePool; }
		[[nodiscard]] auto GetWritableImagePool() const noexcept { return _writableImagePool; }
		[[nodiscard]] auto GetSamplerPool() const noexcept { return _samplerPool; }

		[[nodiscard]] auto GetPipelineLayout() const noexcept { return _pipelineLayout; }

		void GlobalDescriptor();
	private:
		void CreatePipelineLayout();
		void CreateGlobalDescriptorPoolAndSet();
		void CreateBindlessDescriptorPoolAndSet(
			VkDescriptorType type, uint32_t descriptorCount, bool partiallyBound, std::string_view debugName,
			VkDescriptorPool& outPool, VkDescriptorSetLayout& outSetLayout, VkDescriptorSet& outSet);
		void WriteDescriptor(VkDescriptorSet set, VkDescriptorType type, const VkDescriptorImageInfo *imageInfo, uint32_t index) noexcept;

		uint32_t _readonlyImageUsageCount{};
		uint32_t _writableImageUsageCount{};
		uint32_t _samplerUsageCount{};

		GEVector<uint32_t> _readonlyImageDeletedIndices;
		GEVector<uint32_t> _writableImageDeletedIndices;
		GEVector<uint32_t> _samplerDeletedIndices;

		VkDescriptorSetLayout _globalDescriptorSetLayout{VK_NULL_HANDLE};
		VkDescriptorSetLayout _readonlyImageSetLayout{VK_NULL_HANDLE};
		VkDescriptorSetLayout _writableImageSetLayout{VK_NULL_HANDLE};
		VkDescriptorSetLayout _samplerSetLayout{VK_NULL_HANDLE};

		VkDescriptorSet _globalDescriptorSet{VK_NULL_HANDLE};
		VkDescriptorSet _readonlyImageSet{VK_NULL_HANDLE};
		VkDescriptorSet _writableImageSet{VK_NULL_HANDLE};
		VkDescriptorSet _samplerSet{VK_NULL_HANDLE};

		VkDescriptorPool _globalDescriptorPool{VK_NULL_HANDLE};
		VkDescriptorPool _readonlyImagePool{VK_NULL_HANDLE};
		VkDescriptorPool _writableImagePool{VK_NULL_HANDLE};
		VkDescriptorPool _samplerPool{VK_NULL_HANDLE};

		VkPipelineLayout _pipelineLayout{VK_NULL_HANDLE};
	};

	inline Vulkan_DescriptorManagerDefault::~Vulkan_DescriptorManagerDefault() {
		vkDestroyPipelineLayout(_renderContext.GetDevice(), _pipelineLayout, VK_ALLOCATOR_CALLBACKS);

		vkDestroyDescriptorSetLayout(_renderContext.GetDevice(), _globalDescriptorSetLayout, VK_ALLOCATOR_CALLBACKS);
		vkDestroyDescriptorSetLayout(_renderContext.GetDevice(), _readonlyImageSetLayout, VK_ALLOCATOR_CALLBACKS);
		vkDestroyDescriptorSetLayout(_renderContext.GetDevice(), _writableImageSetLayout, VK_ALLOCATOR_CALLBACKS);
		vkDestroyDescriptorSetLayout(_renderContext.GetDevice(), _samplerSetLayout, VK_ALLOCATOR_CALLBACKS);

		vkDestroyDescriptorPool(_renderContext.GetDevice(), _globalDescriptorPool, VK_ALLOCATOR_CALLBACKS);
		vkDestroyDescriptorPool(_renderContext.GetDevice(), _readonlyImagePool, VK_ALLOCATOR_CALLBACKS);
		vkDestroyDescriptorPool(_renderContext.GetDevice(), _writableImagePool, VK_ALLOCATOR_CALLBACKS);
		vkDestroyDescriptorPool(_renderContext.GetDevice(), _samplerPool, VK_ALLOCATOR_CALLBACKS);
	}
}
