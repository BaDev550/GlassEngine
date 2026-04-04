#include "gepch.h"
#include "Buffer.h"

#include "Platform/Vulkan/Vulkan_Buffer.h"

namespace ge::renderer {
	ge::mem::Ref<Buffer> Buffer::Create(uint64_t size, BufferUsageFlags usage, MemoryPropertiesFlags memoryProperties)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Buffer>::Create(size, usage, memoryProperties);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}