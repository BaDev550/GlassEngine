#include "gepch.h"
#include "Texture.h"

#include "Platform/Vulkan/Vulkan_Texture.h"

namespace ge::renderer {
	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Texture2D>::Create(spec);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}

	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec, const std::filesystem::path& filePath)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Texture2D>::Create(spec, filePath);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}