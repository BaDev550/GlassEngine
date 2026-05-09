#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Renderer/RenderAPI.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Types.h"
#include "gepch.h"
#include "RenderPass.h"

namespace ge::renderer {
	void RenderPass::Begin(uint32_t layer) {
		Renderer3D::GetRenderAPI()->BeginDebugLabel(_debugName);

		auto& window = Engine::Get().GetApplicationWindow();
		if (_framebuffer->HasSwapchainImage() && window.HasResized()) {
			_framebuffer->Resize(window.GetSwapchain().GetSpecs().extent.x, window.GetSwapchain().GetSpecs().extent.y);
		}

		BeginRenderPassSpec beginSpec{};
		
		GEVector<Attachment> colorAttachments;
		colorAttachments.reserve(_framebuffer->GetAttachmentCount());
	
		beginSpec.extent = {_framebuffer->GetSpecRef().width, _framebuffer->GetSpecRef().height};
		for (const auto &attachment : _framebuffer->GetAttachments()) {
			if (attachment.isSwapchain) {
				colorAttachments.emplace_back(
					nullptr,
					ImageSubresource{},
					attachment.clearValue,
					attachment.loadOp,
					attachment.storeOp,
					true
				);
			}
			else if (utility::IsColorFormat(attachment.existingImage->GetSpecRef().imageFormat)) {
				colorAttachments.emplace_back(
					attachment.existingImage,
					attachment.subresource,
					attachment.clearValue,
					attachment.loadOp,
					attachment.storeOp,
					false
				);
			}
			else {
				beginSpec.depthStencilAttachment = Attachment{
					attachment.existingImage,
					attachment.subresource,
					attachment.clearValue,
					attachment.loadOp,
					attachment.storeOp,
					false
				};
			}
		}
		beginSpec.colorAttachments = colorAttachments;
		beginSpec.depthWrite = _framebuffer->GetSpecRef().depthWrite;
		beginSpec.stencilWrite = _framebuffer->GetSpecRef().stencilWrite;
		Renderer3D::GetRenderAPI()->BeginRenderPass(beginSpec);
	}

	void RenderPass::End(uint32_t layer) {
		Renderer3D::GetRenderAPI()->EndRenderPass();
		Renderer3D::GetRenderAPI()->EndDebugLabel();
	}
}