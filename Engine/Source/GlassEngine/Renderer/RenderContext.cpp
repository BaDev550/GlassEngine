#include "gepch.h"
#include "RenderContext.h"

#include "Platform/Vulkan/Vulkan_RenderContext.h"

namespace ge::renderer {
	ge::mem::Scope<RenderContext> RenderContext::Create(GLFWwindow* window) {
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::CreateScope<Vulkan_RenderContext>(window);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}