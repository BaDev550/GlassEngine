#pragma once

#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Swapchain.h"
#include "Vulkan_RenderContext.h"

#include <span>

namespace ge::renderer {
	class Vulkan_Swapchain : public Swapchain {
	public:
		Vulkan_Swapchain(const SwapchainSpec& spec, RenderContext& renderContext);
		~Vulkan_Swapchain();

		VkResult Submit(VkCommandBuffer* cmd, uint32_t* imageIndex);
		VkFormat GetSwapchainFormat() const noexcept { return _format; }
		virtual bool Swapbuffers(uint32_t* imageIndex = nullptr) override;
		[[nodiscard]] auto GetSwapchain() const noexcept { return _swapchain; }
		[[nodiscard]] auto GetImageAvailableSemaphores() const noexcept { return std::span(_imageAvailableSemaphores); }
		[[nodiscard]] auto GetRenderFinishedSemaphores() const noexcept { return std::span(_renderFinishedSemaphores); }
		[[nodiscard]] auto GetImageViews() const noexcept { return std::span(_imageViews); }
		[[nodiscard]] auto GetImages() const noexcept { return std::span(_images); }
	private:
		void CreateSwapchain(const SwapchainSpec& newSpec) override;

		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

		GEVector<VkSemaphore> _renderFinishedSemaphores;
		std::array<VkSemaphore, Renderer3D::MaxFramesInFlight> _imageAvailableSemaphores;
		std::array<VkFence, Renderer3D::MaxFramesInFlight> _inFlightFences;

		GEVector<VkImageView> _imageViews;
		GEVector<VkImage> _images;
		VkFormat _format;
		VkPresentModeKHR _presentMode;
		bool reSized{};
	};
}