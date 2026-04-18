#include "gepch.h"
#include "Vulkan_RenderPass.h"
#include "Vulkan_Swapchain.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_Sampler.h"
#include "GlassEngine/Renderer/Renderer.h"

namespace ge::renderer {
	namespace utils {
		static void transitionImageLayout(VkCommandBuffer cmd,
			VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
			VkAccessFlags srcAccess, VkAccessFlags dstAccess) {
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = srcAccess;
			barrier.dstAccessMask = dstAccess;

			vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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
		glm::vec3 clearValue = framebufferSpecs.clearColor;

		uint32_t activeAttachmentCount = framebufferSpecs.IsSwapchain ? 1 : targetFramebuffer->GetAttachmentCount();
		GEVector<VkRenderingAttachmentInfo> colorAttachments(activeAttachmentCount);
		//VkRenderingAttachmentInfo depthAttachment;
		VkExtent2D extent;

		for (int i = 0; i < activeAttachmentCount; i++) {
			VkImage targetImage = VK_NULL_HANDLE;
			if (framebufferSpecs.IsSwapchain) {
				Vulkan_Swapchain* swapchain = static_cast<Vulkan_Swapchain*>(&window.GetSwapchain());
				uint32_t imageIndex = window.GetImageIndex();
				colorAttachments[i].imageView = swapchain->GetImageViews()[imageIndex];
				colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				targetImage = swapchain->GetImages()[imageIndex];
				extent = VkExtent2D({ swapchain->GetExtent().x, swapchain->GetExtent().y });
			}
			else {
				auto image = targetFramebuffer->GetColorAttachmentTexture(i).Cast<Vulkan_Image>();
				colorAttachments[i].imageView = image->CreateGetImageView(ImageSubresource());
				colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				targetImage = image->GetImage();
				extent = VkExtent2D({ framebufferSpecs.width, framebufferSpecs.height });
			}
			colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachments[i].clearValue = VkClearValue({ clearValue.x, clearValue.g, clearValue.b, 1.0f });

			utils::transitionImageLayout(
				cmd,
				targetImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			);
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
				utils::transitionImageLayout(
					cmd,
					targetImage,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					0
				);
			}
			else {
				auto image = targetFramebuffer->GetColorAttachmentTexture(i).Cast<Vulkan_Image>();
				VkImage targetImage = image->GetImage();
				utils::transitionImageLayout(
					cmd,
					targetImage,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT
				);
			}
		}
	}

	void Vulkan_RenderPass::ISetInput(const ShaderResource& resource, const ge::mem::Ref<Buffer>& buffer, uint16_t firstElement, uint16_t elementCount, uint16_t resourceIndex) {
		_descriptorManager->SetBuffer(resource, *buffer.Cast<Vulkan_Buffer>(), firstElement, elementCount, resourceIndex);
	}
	void Vulkan_RenderPass::ISetInput(const ShaderResource& resource, const ge::mem::Ref<Sampler>& sampler, uint16_t resourceIndex) {
		_descriptorManager->SetSampler(resource, *sampler.Cast<Vulkan_Sampler>(), resourceIndex);
	}
	void Vulkan_RenderPass::ISetInput(const ShaderResource& resource, const ge::mem::Ref<Image>& texture, ImageSubresource subresource, uint16_t resourceIndex) {
		_descriptorManager->SetImage(resource, *texture.Cast<Vulkan_Image>(), subresource, resourceIndex);
	}
}