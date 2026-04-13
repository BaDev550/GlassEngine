#include "Sampler.h"

#include "Platform/Vulkan/Vulkan_Sampler.h"

namespace ge::renderer {
	ge::mem::Ref<Sampler> Sampler::Create(const SamplerSpec& desc)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Sampler>::Create(desc);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}