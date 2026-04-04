#include "gepch.h"
#include "ImGuiLayer.h"

#include "Platform/Vulkan/Vulkan_ImGuiLayer.h"

namespace ge {
	ImGuiLayer* ImGuiLayer::Create() {
		switch (renderer::RenderAPI::GetAPI())
		{
		case renderer::GraphicsAPI::Vulkan: return new Vulkan_ImGuiLayer();
		case renderer::GraphicsAPI::DirectX11: return nullptr;
		case renderer::GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}