#include "gepch.h"
#include "Buffer.h"

#include "Platform/Vulkan/Vulkan_Buffer.h"

namespace ge::renderer {
	ge::mem::Ref<Buffer> Buffer::Create(const BufferSpec& desc)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Buffer>::Create(desc);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}