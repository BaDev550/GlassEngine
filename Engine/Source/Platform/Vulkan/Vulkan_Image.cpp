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

	Vulkan_Image::~Vulkan_Image() {
	}

	VkImageView Vulkan_Image::CreateGetImageView(ImageSubresource subresource) noexcept {
		auto &imageView = m_image_views[subresource];

		if (imageView == VK_NULL_HANDLE)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.format = VK_FORMAT_A1B5G5R5_UNORM_PACK16;
			createInfo.image = _image;
			
			VK_RENDER_CONTEXT->GetDevice();
			vkCreateImageView(VK_RENDER_CONTEXT->GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &imageView);
		}

		return imageView;
	}
}