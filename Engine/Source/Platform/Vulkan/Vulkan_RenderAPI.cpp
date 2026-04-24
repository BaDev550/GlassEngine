#include "GlassEngine/Renderer/Swapchain.h"
#include "gepch.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_Swapchain.h"
#include <vulkan/vulkan_core.h>

namespace ge::renderer {
	Vulkan_RenderAPI::Vulkan_RenderAPI()
	{
		for (uint32_t i = 0; i < Renderer3D::MaxFramesInFlight; i++) { // TEMP
			auto& frame = _frames[i];

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = VK_RENDER_CONTEXT->GetGraphicsQueueFamilyIndex();
			vkCreateCommandPool(VK_RENDER_CONTEXT->GetDevice(), &poolInfo, VK_ALLOCATOR_CALLBACKS, &frame.commandPool);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandBufferCount = 1;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = frame.commandPool;
			vkAllocateCommandBuffers(VK_RENDER_CONTEXT->GetDevice(), &allocInfo, &frame.commandBuffer);
		}
	}

	Vulkan_RenderAPI::~Vulkan_RenderAPI()
	{
		for (uint32_t i = 0; i < Renderer3D::MaxFramesInFlight; i++) {
			vkFreeCommandBuffers(VK_RENDER_CONTEXT->GetDevice(), _frames[i].commandPool, 1, &_frames[i].commandBuffer);
			vkDestroyCommandPool(VK_RENDER_CONTEXT->GetDevice(), _frames[i].commandPool, VK_ALLOCATOR_CALLBACKS);
		}
	}

	void Vulkan_RenderAPI::BeginFrame()
	{
		GE_ASSERT(!_frameStarted, "Cannot call beginFrame while processing a frame");
		_frameStarted = true;
		uint32_t frameIndex = Renderer3D::GetFrameIndex();
		FrameContext& frame = _frames[frameIndex];

		Application::Get()->GetWindow().Swapbuffers();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandPool(VK_RENDER_CONTEXT->GetDevice(), frame.commandPool, 0);
		vkBeginCommandBuffer(frame.commandBuffer, &beginInfo);

		auto& window = Application::Get()->GetWindow();
		const auto &swapchain = static_cast<Vulkan_Swapchain &>(window.GetSwapchain());
		const auto image = swapchain.GetImages()[window.GetImageIndex()];

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = frameIndex <= swapchain.GetImages().size() ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			frame.commandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void Vulkan_RenderAPI::EndFrame()
	{
		GE_ASSERT(_frameStarted, "Cannot call endFrame while not processing a frame");
		VkCommandBuffer cmd = GetCurrentCommandBuffer();
		auto& window = Application::Get()->GetWindow();
		auto &swapchain = static_cast<Vulkan_Swapchain &>(window.GetSwapchain());
		uint32_t frameIndex = Renderer3D::GetFrameIndex();
		uint32_t imageIndex = window.GetImageIndex();

		const auto image = swapchain.GetImages()[window.GetImageIndex()];

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			cmd,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vkEndCommandBuffer(cmd);
		VkResult result = swapchain.Submit(&cmd, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			//swapchain->ReCreateSwapchain();
		}
		_renderStats.drawCalls = 0;
		_frameStarted = false;
	}

	void Vulkan_RenderAPI::DrawVertex()
	{
	}
	void Vulkan_RenderAPI::DrawIndexed()
	{
	}

	VkCommandBuffer Vulkan_RenderAPI::GetCurrentCommandBuffer()
	{
		GE_ASSERT(_frameStarted, "Cannot get active command buffer while frame is not started");
		return _frames[Renderer3D::GetFrameIndex()].commandBuffer;
	}
}