#include "gepch.h"
#include "Framebuffer.h"

#include "Platform/Vulkan/Vulkan_Framebuffer.h"

namespace ge::renderer {
	mem::Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec) {
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Framebuffer>::Create(spec);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}