#include "gepch.h"
#include "RenderContext.h"

#include "Platform/Vulkan/Vulkan_RenderContext.h"

namespace ge::renderer {
	uint32_t RenderContext::GetReadonlyImageHandle(Image& image, ImageSubresource subresource) {
		GE_ASSERT(image.GetSpecRef().usageFlags.Has(ImageUsageFlagsBits::Readonly), "image must br have readonly usage bit");

		return IGetReadonlyImageHandle(image, subresource);
	}

	uint32_t RenderContext::GetWritableImageHandle(Image& image, ImageSubresource subresource) {
		GE_ASSERT(image.GetSpecRef().usageFlags.Has(ImageUsageFlagsBits::Writable), "image must br have readonly usage bit");

		return IGetWritableImageHandle(image, subresource);
	}

	uint32_t RenderContext::GetSamplerHandle(Sampler& sampler) {
		return IGetSamplerHandle(sampler);
	}

	ge::mem::Scope<RenderContext> RenderContext::Create(GLFWwindow* window) {
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::CreateScope<Vulkan_RenderContext>(window);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}