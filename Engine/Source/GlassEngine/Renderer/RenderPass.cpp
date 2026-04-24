#include "gepch.h"
#include "RenderPass.h"

#include "Platform/Vulkan/Vulkan_RenderPass.h"

namespace ge::renderer {
	ge::mem::Ref<RenderPass> RenderPass::Create(const ge::mem::Ref<Framebuffer>& framebuffer, std::string_view debugName)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_RenderPass>::Create(framebuffer, debugName);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}