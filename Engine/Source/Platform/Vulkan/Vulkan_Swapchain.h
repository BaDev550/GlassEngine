#pragma once

#include "GlassEngine/Renderer/Swapchain.h"
#include "Vulkan_RenderContext.h"

#include <span>

namespace ge::renderer {
	class Vulkan_Swapchain : public Swapchain {
	public:
		Vulkan_Swapchain(const SwapchainSpec& spec, RenderContext& renderContext);
		~Vulkan_Swapchain();

		VkResult Submit(VkCommandBuffer* cmd, uint32_t* imageIndex);
		virtual bool Swapbuffers(uint32_t* imageIndex = nullptr) override;
		[[nodiscard]] auto GetSwapchain() const noexcept { return _swapchain; }
		[[nodiscard]] auto GetSemaphores() const noexcept { return std::span(_semaphores); }
		[[nodiscard]] auto GetImageViews() const noexcept { return std::span(_imageViews); }
		[[nodiscard]] auto GetImages() const noexcept { return std::span(_images); }
	private:
		void CreateSwapchain(const SwapchainSpec& newSpec) override;

		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		GEVector<VkSemaphore> _semaphores;
		GEVector<VkImageView> _imageViews;
		GEVector<VkImage> _images;
		VkFormat _format;
		VkPresentModeKHR _presentMode;
	};

	inline Vulkan_Swapchain::~Vulkan_Swapchain() {
		vkDestroySwapchainKHR(VK_RENDER_CONTEXT->GetDevice(), _swapchain, VK_ALLOCATOR_CALLBACKS);

		for (auto imageView : _imageViews) {
			vkDestroyImageView(VK_RENDER_CONTEXT->GetDevice(), imageView, VK_ALLOCATOR_CALLBACKS);
		}
	}
}