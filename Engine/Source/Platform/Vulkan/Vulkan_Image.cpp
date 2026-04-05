#include "gepch.h"
#include "Vulkan_Image.h"

namespace ge::renderer {
	static constexpr VkImageType GetVkImageType(ImageType imageType) noexcept {
		switch (imageType) {
		case ImageType::e1D: return VK_IMAGE_TYPE_1D;
		case ImageType::e2D: return VK_IMAGE_TYPE_2D;
		case ImageType::e3D: return VK_IMAGE_TYPE_3D;
		}
	}

	static constexpr VkImageUsageFlags GetVkUsageFlags(ImageUsageFlags usageFlags) noexcept {
		VkImageUsageFlags out;
		if (usageFlags.Has(ImageUsageFlagsBits::ColorAttachment))
			out |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::DepthStencilAttachment))
			out |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::Readonly))
			out |= VK_IMAGE_USAGE_SAMPLED_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::Writable))
			out |= VK_IMAGE_USAGE_STORAGE_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::TransferSrc))
			out |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::TransferDst))
			out |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		return out;
	}

	// TODO:
	static constexpr VkFormat GetVkFormat() {
		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	Vulkan_Image::Vulkan_Image(const ImageCreateDesc& desc)
		: Image(desc)
	{
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
		imageCreateInfo.imageType = GetVkImageType(_desc.imageType);
		imageCreateInfo.format = GetVkFormat();
		imageCreateInfo.mipLevels = _desc.mipmapCount;
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.flags = createFlags;
		imageCreateInfo.usage = GetVkUsageFlags(_desc.usageFlags);

		// imageCreateInfo.samples = VK_RENDER_CONTEXT.GetSampleCount(_desc.sampleCount);
		imageCreateInfo.samples = VK_RENDER_CONTEXT.GetSampleCount();

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		VmaAllocationInfo allocInfo{};

		VK_RENDER_CONTEXT.GetAllocator().AllocateImage(imageCreateInfo, allocCreateInfo, _image, _allocation, allocInfo);
	}

	Vulkan_Image::~Vulkan_Image() {
	}

	VkImageView Vulkan_Image::CreateGetImageView(ImageSubresource subresource) noexcept {
		auto &imageView = m_image_views[subresource];

		if (imageView == VK_NULL_HANDLE)
		{
			VkImageViewCreateInfo createInfo{};

			VK_RENDER_CONTEXT.GetDevice();
			vkCreateImageView(VK_RENDER_CONTEXT.GetDevice(), &createInfo, &mem::Vulkan_AllocatorCallbacks::GetCallbacks(), &imageView);
		}

		return imageView;
	}
}