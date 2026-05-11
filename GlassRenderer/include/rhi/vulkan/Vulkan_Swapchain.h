#pragma once

#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Swapchain.h"

#include <cstdint>
#include <span>

namespace ge::renderer {
	class Vulkan_Swapchain : public Swapchain {
	public:
		Vulkan_Swapchain(const SwapchainSpec& spec, RenderContext& renderContext);
		~Vulkan_Swapchain();

		VkResult Submit(VkCommandBuffer* cmd);
		VkFormat GetSwapchainFormat() const noexcept { return _format; }
		virtual bool Swapbuffers() override;
		[[nodiscard]] auto GetCurrentImage() noexcept { return _images[GetImageIndex()]; }
		[[nodiscard]] auto GetCurrentImageView() noexcept { return _imageViews[GetImageIndex()]; }
		[[nodiscard]] auto GetSwapchain() const noexcept { return _swapchain; }
		[[nodiscard]] auto GetImageAvailableSemaphores() const noexcept { return std::span(_imageAvailableSemaphores); }
		[[nodiscard]] auto GetRenderFinishedSemaphores() const noexcept { return std::span(_renderFinishedSemaphores); }
		[[nodiscard]] auto GetImageViews() const noexcept { return std::span(_imageViews); }
		[[nodiscard]] auto GetImages() const noexcept { return std::span(_images); }
		[[nodiscard]] uint32_t GetImageIndex() noexcept;
	private:
		virtual void CreateSwapchain(const SwapchainSpec& newSpec) override;

		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

		GEVector<VkSemaphore> _renderFinishedSemaphores;
		std::array<VkSemaphore, Renderer3D::MaxFramesInFlight> _imageAvailableSemaphores;
		
		GEVector<VkImageView> _imageViews;
		GEVector<VkImage> _images;
		VkFormat _format;
		VkPresentModeKHR _presentMode;
		uint32_t _imageIndex = -1;
	};
}