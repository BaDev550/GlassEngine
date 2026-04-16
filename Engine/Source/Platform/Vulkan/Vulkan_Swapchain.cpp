#include "Vulkan_Swapchain.h"

namespace ge::renderer {
	Vulkan_Swapchain::Vulkan_Swapchain(const SwapchainSpec& spec, RenderContext& renderContext)
	: Swapchain(spec, renderContext) {

		CreateSwapchain(spec);
	}

	void Vulkan_Swapchain::CreateSwapchain(const SwapchainSpec& newSpec) {
		auto getDesired = [] (auto&& desiredList, auto&& searchList, auto notFindValue) -> auto {
				for (const auto& desiredValue : desiredList)
					for (const auto& value : searchList)
						if (desiredValue == value)
							return value;
				return notFindValue;
			};

		auto getDesiredFunc = [](auto&& desiredList, auto&& searchList, auto notFindValue, auto&& searchListFunc) -> auto {
			for (const auto& desiredValue : desiredList)
				for (const auto& value : searchList) {
					const auto value_ = searchListFunc(value);
					if (desiredValue == value_)
						return value_;
				}
			return notFindValue;
		};

		VkSurfaceTransformFlagBitsKHR currentTransform;
		{
			VkSurfaceCapabilitiesKHR surfaceCapabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VK_RENDER_CONTEXT->GetPhysicalDevice(), VK_RENDER_CONTEXT->GetSurface(), &surfaceCapabilities);

			_imageCount = 3;
			_imageCount = std::clamp(_imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
			currentTransform = surfaceCapabilities.currentTransform;
		}

		{
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(VK_RENDER_CONTEXT->GetPhysicalDevice(), VK_RENDER_CONTEXT->GetSurface(), &formatCount, nullptr);
			GEVector<VkSurfaceFormatKHR> formats(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(VK_RENDER_CONTEXT->GetPhysicalDevice(), VK_RENDER_CONTEXT->GetSurface(), &formatCount, formats.data());

			constexpr auto desiredFormatLinear = std::array{
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_FORMAT_B8G8R8A8_UNORM,
			};

			constexpr auto desiredFormatSrgb = std::array{
				VK_FORMAT_R8G8B8A8_SRGB,
				VK_FORMAT_B8G8R8A8_SRGB,
			};

			const auto desiredFormats = std::span(_spec.srgb ? desiredFormatSrgb : desiredFormatLinear);
			
			_format = getDesiredFunc(desiredFormats, formats, VK_FORMAT_UNDEFINED, [] (VkSurfaceFormatKHR surfaceFormat) -> auto {
				return surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR ? surfaceFormat.format : VK_FORMAT_UNDEFINED;
			});
			// TODO (dnm): error for undefiened format
		}

		{
			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(VK_RENDER_CONTEXT->GetPhysicalDevice(), VK_RENDER_CONTEXT->GetSurface(), &presentModeCount, nullptr);
			GEVector<VkPresentModeKHR> presentMode(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(VK_RENDER_CONTEXT->GetPhysicalDevice(), VK_RENDER_CONTEXT->GetSurface(), &presentModeCount, presentMode.data());

			constexpr auto desiredPresentMode = std::array{
				VK_PRESENT_MODE_MAILBOX_KHR,
				VK_PRESENT_MODE_FIFO_RELAXED_KHR,
				VK_PRESENT_MODE_FIFO_KHR
			};

			if (_spec.vsync) {
				_presentMode = getDesired(desiredPresentMode, presentMode, VK_PRESENT_MODE_FIFO_KHR);
			}
			else {
				_presentMode = getDesired(std::array{ VK_PRESENT_MODE_IMMEDIATE_KHR }, presentMode, VK_PRESENT_MODE_FIFO_KHR);
			}
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.imageArrayLayers = 1;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.oldSwapchain = _swapchain;
		createInfo.surface = VK_RENDER_CONTEXT->GetSurface();
		createInfo.imageExtent = { _spec.extent.x, _spec.extent.y };
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.minImageCount = _imageCount;
		createInfo.imageFormat = _format;
		createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		createInfo.presentMode = _presentMode;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.clipped = VK_TRUE;
		createInfo.preTransform = currentTransform;

		vkCreateSwapchainKHR(VK_RENDER_CONTEXT->GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &_swapchain);

		{
			vkGetSwapchainImagesKHR(VK_RENDER_CONTEXT->GetDevice(), _swapchain, &_imageCount, nullptr);
			_images.resize(_imageCount);
			vkGetSwapchainImagesKHR(VK_RENDER_CONTEXT->GetDevice(), _swapchain, &_imageCount, _images.data());

			for (auto image : _images) {
				VkImageSubresourceRange subresource{};
				subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subresource.baseArrayLayer = 0;
				subresource.baseMipLevel = 0;
				subresource.layerCount = 1;
				subresource.levelCount = 1;

				VkImageViewCreateInfo imageViewCreateInfo{};
				imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				imageViewCreateInfo.format = _format;
				imageViewCreateInfo.image = image;
				imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
				imageViewCreateInfo.subresourceRange = subresource;

				VkImageView imageView;
				vkCreateImageView(VK_RENDER_CONTEXT->GetDevice(), &imageViewCreateInfo, VK_ALLOCATOR_CALLBACKS, &imageView);
				_imageViews.push_back(imageView);
			}
		}
	}
}