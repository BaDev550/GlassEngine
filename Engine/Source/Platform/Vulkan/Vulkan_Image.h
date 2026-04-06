#pragma once

#include "GlassEngine/Renderer/Image.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_Image final : public Image {
	public:
		Vulkan_Image(const ImageCreateDesc& desc);
		virtual ~Vulkan_Image();

		[[nodiscard]] VkImageView CreateGetImageView(ImageSubresource subresource) noexcept;
	private:
		VkImage _image;
		VmaAllocation _allocation;
		VkImageLayout _imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkFormat _format;
		VkImageAspectFlags _aspectFlags;
		
		std::unordered_map<ImageSubresource, VkImageView> m_image_views;
	};
}