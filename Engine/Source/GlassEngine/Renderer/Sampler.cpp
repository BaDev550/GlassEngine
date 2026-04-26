#include "Sampler.h"

#include "Platform/Vulkan/Vulkan_Sampler.h"

namespace ge::renderer {
	uint32_t Sampler::GetHandle() {
		if (_handle == static_cast<uint32_t>(-1)) {
			_handle = _renderContext.GetSamplerHandle(*this);
		}
		return _handle;
	}

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