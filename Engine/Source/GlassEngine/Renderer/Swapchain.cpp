#include "gepch.h"
#include "Swapchain.h"

#include "Platform/Vulkan/Vulkan_Swapchain.h"

namespace ge::renderer {
	ge::mem::Scope<Swapchain> Swapchain::Create(const SwapchainSpec& desc, RenderContext& renderContext)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::CreateScope<Vulkan_Swapchain>(desc, renderContext);
		case GraphicsAPI::DirectX12: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}