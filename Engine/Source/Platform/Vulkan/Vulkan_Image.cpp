#include "gepch.h"
#include "Vulkan_Image.h"
#include "Vulkan_Types.h"

namespace ge::renderer {
	Vulkan_Image::Vulkan_Image(const ImageSpec& desc)
		: Image(desc)
	{
		_format = utility::Vulkan_GetImageFormat(_desc.imageFormat);
		_aspectFlags = utility::Vulkan_GetAspectFlags(_desc.imageFormat);

		VkImageCreateFlags createFlags{};

		if (desc.imageType == ImageType::e2D && (desc.arrayCount % 6) == 0)
			createFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		if (desc.imageType == ImageType::e3D)
			createFlags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.arrayLayers = _desc.arrayCount;
		imageCreateInfo.extent = VkExtent3D{_desc.extent.x, _desc.extent.y, _desc.extent.z };
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.imageType = utility::Vulkan_GetImageType(_desc.imageType);
		imageCreateInfo.format = _format;
		imageCreateInfo.mipLevels = _desc.mipmapCount;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.flags = createFlags;
		imageCreateInfo.usage = utility::Vulkan_GetImageUsageFlags(_desc.usageFlags);
		imageCreateInfo.samples = VK_RENDER_CONTEXT->GetSampleCount(_desc.sampleCount);

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		VmaAllocationInfo allocInfo{};

		VK_ALLOCATOR.AllocateImage(imageCreateInfo, allocCreateInfo, _image, _allocation, allocInfo);
	}

	VkImageView Vulkan_Image::CreateGetImageView(ImageSubresource subresource) noexcept {
		auto &imageView = m_image_views[subresource];

		if (imageView == VK_NULL_HANDLE)
		{
			// TODO (dnm):
			VkImageSubresourceRange vkSubresource{};
			vkSubresource.aspectMask = _aspectFlags;
			vkSubresource.baseArrayLayer = subresource.baseLayer;
			vkSubresource.baseMipLevel = subresource.baseMipmap;
			vkSubresource.layerCount = subresource.layerCount;
			vkSubresource.levelCount = subresource.mipmapCount;

			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.format = _format;
			createInfo.image = _image;
			createInfo.viewType = utility::Vulkan_ImageViewType(subresource.type);
			createInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
			createInfo.subresourceRange = vkSubresource;

			vkCreateImageView(VK_RENDER_CONTEXT->GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &imageView);
		}

		return imageView;
	}

	void Vulkan_Image::SetDebugName(GEString name) const noexcept {
		VkDebugUtilsObjectNameInfoEXT nameInfo;
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectHandle = reinterpret_cast<uint64_t>(_image);
		nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		nameInfo.pObjectName = name.c_str();

		vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
	}
}