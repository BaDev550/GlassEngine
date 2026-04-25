#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Renderer/Swapchain.h"
#include "Platform/Vulkan/Vulkan_RenderContext.h"
#include "gepch.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_Swapchain.h"
#include "Vulkan_Image.h"
#include "Vulkan_Buffer.h"
#include "Vulkan_Sampler.h"
#include <vulkan/vulkan_core.h>

namespace ge::renderer {
	namespace utility {
		constexpr VkPipelineStageFlags2 Vulkan_GetPipelineStageFlagsFromLayout(VkImageLayout layout) noexcept {
			switch (layout) {
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			case VK_IMAGE_LAYOUT_GENERAL:
				return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			default:
				return {};
			}
		}
	
		constexpr VkAccessFlags2 Vulkan_GetAccessFlagsFromLayout(VkImageLayout layout) noexcept {
			switch (layout) {
				case VK_IMAGE_LAYOUT_GENERAL: 
					return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					return VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
					return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					return VK_ACCESS_2_SHADER_READ_BIT;
				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					return VK_ACCESS_2_TRANSFER_READ_BIT;
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					return VK_ACCESS_2_TRANSFER_WRITE_BIT;
				default: return {};
			}
		}
	}

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
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = {};
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

	void Vulkan_RenderAPI::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		vkCmdDraw(GetCurrentCommandBuffer(), vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Vulkan_RenderAPI::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance, int32_t vertexOffset) {
		vkCmdDrawIndexed(GetCurrentCommandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void Vulkan_RenderAPI::BeginCopyPass() {

	}

	void Vulkan_RenderAPI::EndCopyPass() {
		TransitionImageLayouts();
	}

	VkCommandBuffer Vulkan_RenderAPI::GetCurrentCommandBuffer() {
		GE_ASSERT(_frameStarted, "Cannot get active command buffer while frame is not started");
		return _frames[Renderer3D::GetFrameIndex()].commandBuffer;
	}

	void Vulkan_RenderAPI::TransitionImageLayouts() {
		GEVector<VkImageMemoryBarrier2> barriers;
		barriers.reserve(_image_layout_transition_set.size());

		for (const auto* image : _image_layout_transition_set) {
			const auto newLayout = utility::Vulkan_OptimalImageLayout(image->GetSpecRef().usageFlags);

			barriers.emplace_back(
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				nullptr,
				utility::Vulkan_GetPipelineStageFlagsFromLayout(image->GetImageLayout()),
				utility::Vulkan_GetAccessFlagsFromLayout(image->GetImageLayout()),
				utility::Vulkan_GetPipelineStageFlagsFromLayout(newLayout),
				utility::Vulkan_GetAccessFlagsFromLayout(newLayout),
				image->GetImageLayout(),
				newLayout,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				image->GetImage(),
				VkImageSubresourceRange{
					.aspectMask = image->GetAspectFlags(),
					.baseMipLevel = 0,
					.levelCount = VK_REMAINING_MIP_LEVELS,
					.baseArrayLayer = 0,
					.layerCount =  VK_REMAINING_ARRAY_LAYERS
				}
			);
		}

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pImageMemoryBarriers = barriers.data();
		dependencyInfo.imageMemoryBarrierCount = barriers.size();

		vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		_image_layout_transition_set.clear();
	}

	void Vulkan_RenderAPI::BeginRenderPass(const BeginRenderPassSpec& spec) {
		;	
	}

	void Vulkan_RenderAPI::EndRenderPass() {
		_image_layout_transition_set.clear();
	}

	void Vulkan_RenderAPI::LoadDataToTexture2D(Texture2D& texture, void* data, uint64_t dataSize) {}
	
	void Vulkan_RenderAPI::SetBeginDebugLabel(std::string_view label) {
		VkDebugUtilsLabelEXT label_desc{};
		label_desc.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label_desc.pLabelName = label.data();
		// label_desc.color = {0.8f, 0.2f, 0.6f, 1.f};
		vkCmdBeginDebugUtilsLabelEXT(GetCurrentCommandBuffer(), &label_desc);
	}

	void Vulkan_RenderAPI::SetEndDebugLabel() {

	}
}