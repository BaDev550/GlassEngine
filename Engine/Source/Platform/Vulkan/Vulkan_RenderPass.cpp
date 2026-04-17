#include "gepch.h"
#include "Vulkan_RenderPass.h"
#include "Vulkan_Swapchain.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_Sampler.h"
#include "GlassEngine/Renderer/Renderer.h"

namespace ge::renderer {
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
			if (framebufferSpecs.IsSwapchain) {
				Vulkan_Swapchain* swapchain = static_cast<Vulkan_Swapchain*>(&window.GetSwapchain());
				uint32_t imageIndex = window.GetImageIndex();
				colorAttachments[i].imageView = swapchain->GetImageViews()[imageIndex];
				colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				//extent = swapchain->GetSwapchainExtent();
			}
			else {
				auto image = targetFramebuffer->GetColorAttachmentTexture(i).Cast<Vulkan_Image>();
				colorAttachments[i].imageView = image->CreateGetImageView(ImageSubresource());
				colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				extent = VkExtent2D({ framebufferSpecs.width, framebufferSpecs.height });
			}
			colorAttachments[i].clearValue = VkClearValue({ clearValue.x, clearValue.g, clearValue.b, 1.0f });
		} // TODO (dnm): image memory barrier

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
	}

	void Vulkan_RenderPass::ISetInput(const ShaderResource& resource, const ge::mem::Ref<Buffer>& buffer, uint16_t firstElement, uint16_t elementCount, uint16_t resourceIndex)
	{
		_descriptorManager->SetBuffer(resource, *buffer.Cast<Vulkan_Buffer>(), firstElement, elementCount, resourceIndex);
	}

	void Vulkan_RenderPass::ISetInput(const ShaderResource& resource, const ge::mem::Ref<Sampler>& sampler, uint16_t resourceIndex)
	{
		_descriptorManager->SetSampler(resource, *sampler.Cast<Vulkan_Sampler>(), resourceIndex);
	}

	void Vulkan_RenderPass::ISetInput(const ShaderResource& resource, const ge::mem::Ref<Image>& texture, ImageSubresource subresource, uint16_t resourceIndex)
	{
		_descriptorManager->SetImage(resource, *texture.Cast<Vulkan_Image>(), subresource, resourceIndex);
	}
}