#pragma once

#include "GlassEngine/Renderer/Image.h"
#include "Vulkan_RenderContext.h"

#include <ranges>

namespace ge::renderer {
	class Vulkan_Image final : public Image {
	public:
		Vulkan_Image(const ImageSpec& desc);
		~Vulkan_Image();

		[[nodiscard]] auto GetImage() const noexcept { return _image; }
		[[nodiscard]] auto GetFormat() const noexcept { return _format; }
		[[nodiscard]] auto GetAspectFlags() const noexcept { return _aspectFlags; }
		[[nodiscard]] auto GetImageLayout() const noexcept { return _imageLayout; }
		[[nodiscard]] VkImageView CreateGetImageView(ImageSubresource subresource) noexcept;

		virtual void SetDebugName(GEString name) const noexcept override;
	private:
		VkImage _image;
		VmaAllocation _allocation;
		VkFormat _format;
		VkImageAspectFlags _aspectFlags;
		// for sync
		VkImageLayout _imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkAccessFlags2 _accessFlags;
		VkPipelineStageFlags2 _pipelineStageFlags;
		
		std::unordered_map<ImageSubresource, VkImageView> m_image_views;
		friend class Vulkan_RenderAPI;
	};

	inline Vulkan_Image::~Vulkan_Image() {
		for (auto imageView : m_image_views | std::ranges::views::values)
			vkDestroyImageView(VK_RENDER_CONTEXT->GetDevice(), imageView, VK_ALLOCATOR_CALLBACKS);

		VK_ALLOCATOR.DestroyImage(_image, _allocation);
	}
}