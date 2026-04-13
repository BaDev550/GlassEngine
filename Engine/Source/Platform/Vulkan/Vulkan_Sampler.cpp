#include "gepch.h"
#include "Vulkan_Sampler.h"
#include "Vulkan_Types.h"

namespace ge::renderer {
	Vulkan_Sampler::Vulkan_Sampler(const SamplerSpec& desc)
		: Sampler(desc) {
		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.addressModeU = utility::Vulkan_GetSamplerAddressMode(_desc.addressMode);
		createInfo.addressModeW = utility::Vulkan_GetSamplerAddressMode(_desc.addressMode);
		createInfo.addressModeV = utility::Vulkan_GetSamplerAddressMode(_desc.addressMode);
		createInfo.compareOp = utility::Vulkan_GetCompareOp(_desc.compareOp);
		createInfo.magFilter = utility::Vulkan_GetFilterType(_desc.filterType);
		createInfo.minFilter = utility::Vulkan_GetFilterType(_desc.filterType);
		createInfo.mipmapMode = utility::Vulkan_GetMipmapFilterType(_desc.filterType);
		createInfo.maxLod = desc.maxLod;
		createInfo.minLod = desc.minLod;
		createInfo.mipLodBias = desc.lodBias;
		createInfo.compareEnable = _desc.compareOp == CompareOp::None;
		createInfo.maxAnisotropy = _desc.anisotropy;
		createInfo.anisotropyEnable = _desc.anisotropy == 1.f;
		createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		createInfo.unnormalizedCoordinates = false;

		vkCreateSampler(VK_RENDER_CONTEXT->GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &_sampler);
	}

	Vulkan_Sampler::~Vulkan_Sampler() {
		vkDestroySampler(VK_RENDER_CONTEXT->GetDevice(), _sampler, VK_ALLOCATOR_CALLBACKS);
	}
}