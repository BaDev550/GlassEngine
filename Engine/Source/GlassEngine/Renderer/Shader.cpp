#include "gepch.h"
#include "Shader.h"

#include "Platform/Vulkan/Vulkan_Shader.h"

namespace ge::renderer {
	ge::mem::Ref<Shader> Shader::Create(const GEString& vertexPath, const GEString& fragmentPath) {
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Shader>::Create(vertexPath, fragmentPath);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}