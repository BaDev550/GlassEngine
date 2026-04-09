#pragma once

#include <GlassEngine/Renderer/ShaderCompiler.h>
#include "Vulkan_Image.h"
#include "Vulkan_Buffer.h"

namespace ge::renderer {
	class Vulkan_DescriptorManager {
	public:
		Vulkan_DescriptorManager(Vulkan_RenderContext& renderContext, const ShaderReflection& reflection);
		
		[[nodiscard]] auto GetDstSetLayout() const noexcept { return _descriptorSetLayout; }
		[[nodiscard]] auto GetDstSet() const noexcept { return _descriptorSet; }
		[[nodiscard]] auto GetPipelineLayout() const noexcept { return _pipelineLayout; }

		void SetImage(
			const ShaderResource& resource, Vulkan_Image& image, ImageSubresource subresource, uint16_t resourceIndex) const noexcept;
		void SetBuffer(
			const ShaderResource& resource, const Vulkan_Buffer& buffer, uint16_t firstElement, uint16_t elementCount, uint16_t resourceIndex) const noexcept;
		// TODO (0x): sampler 
		void SetSampler();
	private:
		VkDescriptorSetLayout _descriptorSetLayout{VK_NULL_HANDLE};
		VkDescriptorSet _descriptorSet{VK_NULL_HANDLE};
		VkPipelineLayout _pipelineLayout{VK_NULL_HANDLE};

		Vulkan_RenderContext& _renderContext;
	};
}