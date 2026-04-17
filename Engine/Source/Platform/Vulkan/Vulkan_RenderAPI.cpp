#include "gepch.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_Swapchain.h"

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

		vkResetCommandPool(VK_RENDER_CONTEXT->GetDevice(), frame.commandPool, 0);
		vkBeginCommandBuffer(frame.commandBuffer, &beginInfo);
	}

	void Vulkan_RenderAPI::EndFrame()
	{
		GE_ASSERT(_frameStarted, "Cannot call endFrame while not processing a frame");
		VkCommandBuffer cmd = GetCurrentCommandBuffer();
		auto& window = Application::Get()->GetWindow();
		auto swapchain = CastChecked<Vulkan_Swapchain>(&window.GetSwapchain());
		uint32_t frameIndex = Renderer3D::GetFrameIndex();
		uint32_t imageIndex = window.GetImageIndex();

		vkEndCommandBuffer(cmd);
		VkResult result = swapchain->Submit(nullptr, &imageIndex);
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