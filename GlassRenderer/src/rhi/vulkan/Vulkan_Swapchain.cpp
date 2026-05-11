#include "Vulkan_Swapchain.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Utilities/Counter.h"
#include "Platform/Vulkan/Vulkan_RenderAPI.h"
#include "Platform/Vulkan/Vulkan_RenderContext.h"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ge::renderer {
	Vulkan_Swapchain::Vulkan_Swapchain(const SwapchainSpec& spec, RenderContext& renderContext)
	: Swapchain(spec, renderContext) {

		CreateSwapchain(spec);
	}

	Vulkan_Swapchain::~Vulkan_Swapchain()
	{
		const auto device = VK_RENDER_CONTEXT->GetDevice();

		vkDeviceWaitIdle(device);

		for (const auto semaphore : _renderFinishedSemaphores) {
			vkDestroySemaphore(device, semaphore, 
				VK_ALLOCATOR_CALLBACKS);
		}

		for (const auto i : Counter(Renderer3D::MaxFramesInFlight)) {
			vkDestroySemaphore(device, _imageAvailableSemaphores[i], VK_ALLOCATOR_CALLBACKS);
		}

		for (const auto imageView : _imageViews) {
			vkDestroyImageView(device, imageView, VK_ALLOCATOR_CALLBACKS);
		}

		vkDestroySwapchainKHR(device, _swapchain, VK_ALLOCATOR_CALLBACKS);
	}

	void Vulkan_Swapchain::CreateSwapchain(const SwapchainSpec& newSpec) {
		GE_PROFILE_SCOPE("Create::RHI_Vulkan_Swapchain");
		VK_RENDER_CONTEXT->Wait();
		const auto device = VK_RENDER_CONTEXT->GetDevice();

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
			_imageCount = std::clamp(_imageCount, 
				surfaceCapabilities.minImageCount, 
				surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : UINT32_MAX);

			currentTransform = surfaceCapabilities.currentTransform;

			if (surfaceCapabilities.currentExtent.width != 0xFFFFFFFF) {
				_spec.extent.x = surfaceCapabilities.currentExtent.width;
				_spec.extent.y = surfaceCapabilities.currentExtent.height;
			}
			else {
				_spec.extent.x = std::clamp(_spec.extent.x, 
					surfaceCapabilities.minImageExtent.width, 
					surfaceCapabilities.maxImageExtent.width);
	
				_spec.extent.y = std::clamp(_spec.extent.y, 
					surfaceCapabilities.minImageExtent.height, 
					surfaceCapabilities.maxImageExtent.height);
			}
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

		auto oldSwapchain = _swapchain;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.oldSwapchain = _swapchain;
		createInfo.imageArrayLayers = 1;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
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

		vkCreateSwapchainKHR(device, &createInfo, VK_ALLOCATOR_CALLBACKS, &_swapchain);

		vkDestroySwapchainKHR(device, oldSwapchain, VK_ALLOCATOR_CALLBACKS);

		{
			vkGetSwapchainImagesKHR(device, _swapchain, &_imageCount, nullptr);
			_images.resize(_imageCount);
			vkGetSwapchainImagesKHR(device, _swapchain, &_imageCount, _images.data());
			
			for (auto imageView : _imageViews) {
				vkDestroyImageView(device, imageView, VK_ALLOCATOR_CALLBACKS);
			}
			_imageViews.clear();

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
				vkCreateImageView(device, &imageViewCreateInfo, VK_ALLOCATOR_CALLBACKS, &imageView);
				_imageViews.push_back(imageView);
			}

			// for (const auto image : _images) {
			// 	VkImageMemoryBarrier barrier{};
			// 	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			// 	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			// 	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			// 	barrier.srcAccessMask = {};
			// 	barrier.dstAccessMask = {};
			// 	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			// 	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			// 	barrier.image = image;
			// 	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			// 	barrier.subresourceRange.baseMipLevel = 0;
			// 	barrier.subresourceRange.levelCount = 1;
			// 	barrier.subresourceRange.baseArrayLayer = 0;
			// 	barrier.subresourceRange.layerCount = 1;

			// 	vkCmdPipelineBarrier(
			// 		Renderer3D::GetRenderAPI().Cast<Vulkan_RenderAPI>()->GetCurrentCommandBuffer(),
			// 		VkPipelineStageFlags{}, VkPipelineStageFlagBits{}, {}, 0, nullptr, 0, nullptr, 
			// 		1, &barrier);
			// }
		}

		if (_renderFinishedSemaphores.empty()) {
			_renderFinishedSemaphores.resize(_imageCount);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (auto& semaphore : _renderFinishedSemaphores) {
				vkCreateSemaphore(device, &semaphoreInfo, 
					VK_ALLOCATOR_CALLBACKS, &semaphore);
			}

			for (const auto i : Counter(Renderer3D::MaxFramesInFlight)) {
				vkCreateSemaphore(device, 
					&semaphoreInfo, VK_ALLOCATOR_CALLBACKS, &_imageAvailableSemaphores[i]);
			}
		}
		else {
			std::vector<VkImageMemoryBarrier> memBarrier;
			memBarrier.reserve(GetImages().size());
			for (const auto image : GetImages()) {
				memBarrier.emplace_back(
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // sType
					nullptr, // pNext
					VkAccessFlags{}, // srcAccessMask
					VkAccessFlags{}, // dstAccessMask
					VK_IMAGE_LAYOUT_UNDEFINED, // oldLayout
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // newLayout
					VK_QUEUE_FAMILY_IGNORED, // srcQueueFamilyIndex
					VK_QUEUE_FAMILY_IGNORED, // dstQueueFamilyIndex
					image, // image
					VkImageSubresourceRange{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1
					} // subresourceRange
				);
			}

			auto renderApi = Renderer3D::GetRenderAPI().Cast<Vulkan_RenderAPI>();

			vkCmdPipelineBarrier(
				renderApi->GetCurrentCommandBuffer(),
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0,
				0, nullptr,
				0, nullptr,
				memBarrier.size(), memBarrier.data()
			);
		}
	}

	uint32_t Vulkan_Swapchain::GetImageIndex() noexcept {
		if (_imageIndex != -1)
			return _imageIndex;

		const uint32_t frameIndex = Renderer3D::GetFrameIndex();

		VkResult result = vkAcquireNextImageKHR(VK_RENDER_CONTEXT->GetDevice(), _swapchain, UINT64_MAX, _imageAvailableSemaphores[frameIndex], VK_NULL_HANDLE, &_imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) { 
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { 
			return false;
		}
		
		return _imageIndex;
	}

	bool Vulkan_Swapchain::Swapbuffers() {
		auto renderApi = Renderer3D::GetRenderAPI().Cast<Vulkan_RenderAPI>();

		const uint32_t frameIndex = Renderer3D::GetFrameIndex();

		const auto currentCommandBuffer = renderApi->GetCurrentCommandBuffer();

		const auto renderContext = CastChecked<Vulkan_RenderContext>(&Engine::Get().GetApplicationWindow().GetRenderContext());

		const VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[frameIndex] };
		const VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_imageIndex] };
		constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &currentCommandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		renderApi->EndCommandBuffer();

		VkResult submitResult = vkQueueSubmit(renderContext->GetGraphicsQueue(), 1, &submitInfo, renderApi->GetCurrentInFlightFence());
		if (submitResult != VK_SUCCESS) {
			return submitResult;
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapchain;
		presentInfo.pImageIndices = &_imageIndex;

		const auto result = vkQueuePresentKHR(renderContext->GetGraphicsQueue(), &presentInfo);

		_imageIndex = -1;

		{
			auto& window = Engine::Get().GetApplicationWindow();
	
			if (window.HasResized()) {
				SwapchainSpec newSpec = GetSpecs();
				newSpec.extent.x = window.GetWidth();
				newSpec.extent.y = window.GetHeight();
				ReCreateSwapchain(newSpec);
				window.ResetResizeFlag();
			}
		}

		return result;
	}
}