#pragma once

#include "GlassEngine/Renderer/Image.h"
#include "Vulkan_RenderContext.h"

#include <ranges>

namespace ge::renderer {
	class Vulkan_Image final : public Image {
	public:
		Vulkan_Image(const ImageSpec& desc);
		~Vulkan_Image();

		[[nodiscard]] VkImageView CreateGetImageView(ImageSubresource subresource) noexcept;
	private:
		VkImage _image;
		VmaAllocation _allocation;
		VkImageLayout _imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkFormat _format;
		VkImageAspectFlags _aspectFlags;
		
		std::unordered_map<ImageSubresource, VkImageView> m_image_views;
	};

	inline Vulkan_Image::~Vulkan_Image() {
		for (auto imageView : m_image_views | std::ranges::views::values)
			vkDestroyImageView(VK_RENDER_CONTEXT->GetDevice(), imageView, VK_ALLOCATOR_CALLBACKS);

		VK_ALLOCATOR.DestroyImage(_image, _allocation);
	}
}