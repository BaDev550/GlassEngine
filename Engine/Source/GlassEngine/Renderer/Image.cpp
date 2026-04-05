#include "Image.h"

#include "Platform/Vulkan/Vulkan_Image.h"

namespace ge::renderer {
	ge::mem::Ref<Image> Image::Create(const ImageCreateDesc& desc)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Image>::Create(desc);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}