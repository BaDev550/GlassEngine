#include "gepch.h"
#include "RenderPass.h"

#include "Platform/Vulkan/Vulkan_RenderPass.h"

namespace ge::renderer {
	ge::mem::Ref<RenderPass> RenderPass::Create(const ge::mem::Ref<Pipeline>& pipeline)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_RenderPass>::Create(pipeline);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}

	void RenderPass::SetInput(std::string_view name, const ge::mem::Ref<Buffer>& buffer, uint16_t firstElement, uint16_t elementCount, uint16_t resourceIndex) {
		const auto* decl = GetShaderResource(name);
		if (decl) {
			ISetInput(*decl, buffer, firstElement, elementCount, resourceIndex);
		}
	}

	void RenderPass::SetInput(std::string_view name, const ge::mem::Ref<Sampler>& sampler, uint16_t resourceIndex) {
		const auto* decl = GetShaderResource(name);
		if (decl) {
			ISetInput(*decl, sampler, resourceIndex);
		}
	}

	void RenderPass::SetInput(std::string_view name, const ge::mem::Ref<Image>& image, ImageSubresource subresource, uint16_t resourceIndex) {
		const auto* decl = GetShaderResource(name);
		if (decl) {
			ISetInput(*decl, image, subresource, resourceIndex);
		}
	}

	const ShaderResource* RenderPass::GetShaderResource(std::string_view name)
	{
		auto& compiledData = _pipeline->GetShader()->GetReflectionData();
		GEString nameStr(name);
		if (compiledData.resources.find(nameStr) == compiledData.resources.end())
			return nullptr;
		const ShaderResource& decl = compiledData.resources.at(nameStr);
		return &decl;
	}
}