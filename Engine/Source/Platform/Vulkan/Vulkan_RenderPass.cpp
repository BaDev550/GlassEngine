#include "gepch.h"
#include "Vulkan_RenderPass.h"
#include "Vulkan_Swapchain.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_Sampler.h"
#include "Vulkan_Image.h"
#include "GlassEngine/Renderer/Renderer.h"

namespace ge::renderer {
	namespace Utils {
		bool HasStencilComponent(VkFormat Format)
		{
			return ((Format == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
				(Format == VK_FORMAT_D24_UNORM_S8_UINT));
		}
		// Copied from the "3D Graphics Rendering Cookbook"
		void ImageMemBarrier(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format, uint32_t mipLevels,
			VkImageLayout OldLayout, VkImageLayout NewLayout, int LayerCount)
		{
			VkImageMemoryBarrier barrier = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = NULL,
				.srcAccessMask = 0,
				.dstAccessMask = 0,
				.oldLayout = OldLayout,
				.newLayout = NewLayout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Image,
				.subresourceRange = VkImageSubresourceRange {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = mipLevels,
					.baseArrayLayer = 0,
					.layerCount = (uint32_t)LayerCount
				}
			};

			VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE;
			VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE;

			if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
				(Format == VK_FORMAT_D16_UNORM) ||
				(Format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
				(Format == VK_FORMAT_D32_SFLOAT) ||
				(Format == VK_FORMAT_S8_UINT) ||
				(Format == VK_FORMAT_D16_UNORM_S8_UINT) ||
				(Format == VK_FORMAT_D24_UNORM_S8_UINT))
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (HasStencilComponent(Format)) {
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}

			if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_GENERAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}

			if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
				NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			} /* Convert back from read-only to updateable */
			else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			} /* Convert from updateable texture to shader read-only */
			else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			} /* Convert depth texture from undefined state to depth-stencil buffer */
			else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			} /* Wait for render pass to complete */
			else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = 0;
				/*
						sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				///		destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
						destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				*/
				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			} /* Convert back from read-only to color attachment */
			else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			} /* Convert from updateable texture to shader read-only */
			else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			} /* Convert back from read-only to depth attachment */
			else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			} /* Convert from updateable depth texture to shader read-only */
			else if (OldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = 0;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			}
			else if (OldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else {
				printf("Unknown barrier case\n");
				exit(1);
			}

			vkCmdPipelineBarrier(CmdBuf, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);
		}
	}

	Vulkan_RenderPass::Vulkan_RenderPass(const ge::mem::Ref<Pipeline>& pipeline)
	{
		_pipeline = pipeline;
	}

	void Vulkan_RenderPass::Begin(uint32_t layer)
	{
		VkCommandBuffer cmd = Renderer3D::GetRenderAPI().Cast<Vulkan_RenderAPI>()->GetCurrentCommandBuffer();
		uint32_t frameIndex = Renderer3D::GetFrameIndex();
		auto pipelineData = _pipeline->GetSpecification();
		auto& targetShader = pipelineData.shader;
		auto& targetFramebuffer = pipelineData.targetFramebuffer;
		auto& framebufferSpecs = targetFramebuffer->GetSpecification();
		auto& window = Application::Get()->GetWindow();
		glm::vec3 clearValue{1.0f, 1.0f, 1.0f};
		float depthClearValue{1.0f};

		uint32_t activeAttachmentCount = framebufferSpecs.IsSwapchain ? 1 : targetFramebuffer->GetAttachmentCount();
		GEVector<VkRenderingAttachmentInfo> colorAttachments(activeAttachmentCount);
		//VkRenderingAttachmentInfo depthAttachment;
		VkExtent2D extent;

		for (int i = 0; i < activeAttachmentCount; i++) {
			VkImage targetImage = VK_NULL_HANDLE;
			VkFormat targetFormat = VK_FORMAT_UNDEFINED;
			if (framebufferSpecs.IsSwapchain) {
				Vulkan_Swapchain* swapchain = static_cast<Vulkan_Swapchain*>(&window.GetSwapchain());
				uint32_t imageIndex = window.GetImageIndex();
				colorAttachments[i].imageView = swapchain->GetImageViews()[imageIndex];
				colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[i].resolveMode = VK_RESOLVE_MODE_NONE;
				colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				targetImage = swapchain->GetImages()[imageIndex];
				targetFormat = swapchain->GetSwapchainFormat();
				extent = VkExtent2D({ swapchain->GetExtent().x, swapchain->GetExtent().y });
			}
			else {
				auto image = targetFramebuffer->GetColorAttachmentTexture(i).Cast<Vulkan_Image>();
				colorAttachments[i].imageView = image->CreateGetImageView(ImageSubresource());
				colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				targetImage = image->GetImage();
				targetFormat = utility::Vulkan_GetImageFormat(image->GetSpec().imageFormat);
				extent = VkExtent2D({ framebufferSpecs.width, framebufferSpecs.height });
				clearValue = framebufferSpecs.Attachments[i].clearColor;
			}
			colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachments[i].clearValue = VkClearValue({ clearValue.x, clearValue.g, clearValue.b, 1.0f });
		}

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = VkRect2D({ 0, 0 }, extent);
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		renderingInfo.pColorAttachments = colorAttachments.data();
		//renderingInfo.pDepthAttachment = 
		
		VkViewport viewport{ 0, 0, (float)extent.width, (float)extent.height };
		viewport.minDepth = 0;
		viewport.maxDepth = 1;
		VkRect2D scissor{ {0,0}, extent };
		vkCmdBeginRendering(cmd, &renderingInfo);
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

	void Vulkan_RenderPass::End(uint32_t layer)
	{
		VkCommandBuffer cmd = Renderer3D::GetRenderAPI().Cast<Vulkan_RenderAPI>()->GetCurrentCommandBuffer();
		vkCmdEndRendering(cmd);

		auto pipelineData = _pipeline->GetSpecification();
		auto& targetFramebuffer = pipelineData.targetFramebuffer;
		auto& framebufferSpecs = targetFramebuffer->GetSpecification();
		auto& window = Application::Get()->GetWindow();

		uint32_t activeAttachmentCount = framebufferSpecs.IsSwapchain ? 1 : targetFramebuffer->GetAttachmentCount();

		for (int i = 0; i < activeAttachmentCount; i++) {
			if (framebufferSpecs.IsSwapchain) {
				Vulkan_Swapchain* swapchain = static_cast<Vulkan_Swapchain*>(&window.GetSwapchain());
				uint32_t imageIndex = window.GetImageIndex();
				VkImage targetImage = swapchain->GetImages()[imageIndex];
				VkFormat targetFormat = swapchain->GetSwapchainFormat();
			}
			else {
				auto image = targetFramebuffer->GetColorAttachmentTexture(i).Cast<Vulkan_Image>();
				VkImage targetImage = image->GetImage();
				VkFormat targetFormat = utility::Vulkan_GetImageFormat(image->GetSpec().imageFormat);
			}
		}
	}
}