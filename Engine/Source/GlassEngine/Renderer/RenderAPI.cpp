#include "gepch.h"
#include "RenderAPI.h"

#include "Platform/Vulkan/Vulkan_RenderAPI.h"

namespace ge::renderer {
	RenderStats RenderAPI::_renderStats;
	GraphicsAPI RenderAPI::_graphicsAPI = GraphicsAPI::Vulkan;
	mem::Ref<RenderAPI> RenderAPI::Create() {
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_RenderAPI>::Create();
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}