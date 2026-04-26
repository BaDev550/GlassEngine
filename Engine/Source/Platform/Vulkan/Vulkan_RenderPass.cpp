#include "gepch.h"
#include "Vulkan_RenderPass.h"
#include "Vulkan_Swapchain.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_Sampler.h"
#include "Vulkan_Image.h"
#include "GlassEngine/Renderer/Renderer.h"

namespace ge::renderer {
	Vulkan_RenderPass::Vulkan_RenderPass(const ge::mem::Ref<Framebuffer>& framebuffer, std::string_view debugName)
		: RenderPass(framebuffer, debugName) {}

	void Vulkan_RenderPass::Begin(uint32_t layer)
	{
		VkCommandBuffer cmd = Renderer3D::GetRenderAPI().Cast<Vulkan_RenderAPI>()->GetCurrentCommandBuffer();
#ifdef _DEBUG
		if (!_debugName.empty()) {
			const auto label = VkDebugUtilsLabelEXT{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
				.pLabelName = _debugName.c_str(),
				.color = { 1.0f, 1.0f, 1.0f, 1.0f }
			};

			vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
		}
#endif
		uint32_t frameIndex = Renderer3D::GetFrameIndex();
		auto& framebufferSpecs = _framebuffer->GetSpecification();
		auto& window = Application::Get()->GetWindow();
		glm::vec3 clearValue{ 1.0f, 0.0f, 0.0f };
		float depthClearValue{ 1.0f };

		uint32_t activeAttachmentCount = framebufferSpecs.IsSwapchain ? 1 : _framebuffer->GetAttachmentCount();
		bool hasDepthAttachment = _framebuffer->HasDepthStencilAttachment();
		GEVector<VkRenderingAttachmentInfo> colorAttachments(activeAttachmentCount);
		VkRenderingAttachmentInfo depthAttachment{};
		VkExtent2D extent;

		if (hasDepthAttachment) {
			auto image = _framebuffer->GetDepthStencilAttachmentTexture().Cast<Vulkan_Image>();
			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			depthAttachment.imageView = image->CreateGetImageView(ImageSubresource{});
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		}

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
				auto image = _framebuffer->GetColorAttachmentTexture(i).Cast<Vulkan_Image>();
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
		if (hasDepthAttachment) {
			renderingInfo.pDepthAttachment = &depthAttachment;
			renderingInfo.pStencilAttachment = &depthAttachment;
		}
		
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

		auto& framebufferSpecs = _framebuffer->GetSpecification();
		auto& window = Application::Get()->GetWindow();

		uint32_t activeAttachmentCount = framebufferSpecs.IsSwapchain ? 1 : _framebuffer->GetAttachmentCount();

		for (int i = 0; i < activeAttachmentCount; i++) {
			if (framebufferSpecs.IsSwapchain) {
				Vulkan_Swapchain* swapchain = static_cast<Vulkan_Swapchain*>(&window.GetSwapchain());
				uint32_t imageIndex = window.GetImageIndex();
				VkImage targetImage = swapchain->GetImages()[imageIndex];
				VkFormat targetFormat = swapchain->GetSwapchainFormat();
			}
			else {
				auto image = _framebuffer->GetColorAttachmentTexture(i).Cast<Vulkan_Image>();
				VkImage targetImage = image->GetImage();
				VkFormat targetFormat = utility::Vulkan_GetImageFormat(image->GetSpec().imageFormat);
			}
		}

		if (!_debugName.empty()) {
			vkCmdEndDebugUtilsLabelEXT(cmd);
		}
	}
}