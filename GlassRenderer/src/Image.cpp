#include "Image.h"

#include "rhi/vulkan/Vulkan_Image.h"

namespace ge::graphics {
	mem::Ref<Image> Image::Create(const ImageSpec& desc)
	{
		switch (GRAPHICS_API_ENUM)
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Image>::Create(desc);
		case GraphicsAPI::DirectX12: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}