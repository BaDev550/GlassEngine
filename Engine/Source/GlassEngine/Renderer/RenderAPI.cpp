#include "GlassEngine/Core/Core.h"
#include "gepch.h"
#include "RenderAPI.h"

#include "Platform/Vulkan/Vulkan_RenderAPI.h"

namespace ge::renderer {
	RenderStats RenderAPI::_renderStats;
	GraphicsAPI RenderAPI::_graphicsAPI = GraphicsAPI::Vulkan;
	ge::mem::Ref<RenderAPI> RenderAPI::Create() {
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_RenderAPI>::Create();
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}

	void RenderAPI::CopyBufferToBuffer(
		const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Buffer>& dst, 
		uint32_t size, uint32_t srcOffset, uint32_t dstOffset
	) {
		GE_ASSERT(src->GetSpecRef().usageFlags.Has(ge::renderer::BufferUsageFlagsBits::TransferSrc), "src has BufferUsageFlagsBits::TransferSrc bit");
		GE_ASSERT(dst->GetSpecRef().usageFlags.Has(ge::renderer::BufferUsageFlagsBits::TransferDst), "dst has BufferUsageFlagsBits::TransferDst bit");		

		ICopyBufferToBuffer(src, dst, size, srcOffset, dstOffset);
	}

	void RenderAPI::CopyBufferToImage(
		const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Image>& dst, 
		const ImageSubresourceLayers& imageSubresource, glm::uvec3 extent, uint32_t srcOffset, glm::uvec3 dstOffset
	) {
		GE_ASSERT(src->GetSpecRef().usageFlags.Has(ge::renderer::BufferUsageFlagsBits::TransferSrc), "src has BufferUsageFlagsBits::TransferSrc bit");
		GE_ASSERT(dst->GetSpecRef().usageFlags.Has(ge::renderer::ImageUsageFlagsBits::TransferDst), "dst has ImageUsageFlagsBits::TransferDst bit");

		ICopyBufferToImage(src, dst, imageSubresource, extent, srcOffset, dstOffset);
	}

	void RenderAPI::CopyImageToBuffer(
		const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& imageSubresource, 
		const ge::mem::Ref<Buffer>& dst, glm::uvec3 extent, glm::uvec3 srcOffset, uint32_t dstOffset
	) {
		GE_ASSERT(src->GetSpecRef().usageFlags.Has(ge::renderer::ImageUsageFlagsBits::TransferSrc), "src has ImageUsageFlagsBits::TransferSrc bit");
		GE_ASSERT(dst->GetSpecRef().usageFlags.Has(ge::renderer::BufferUsageFlagsBits::TransferDst), "dst has BufferUsageFlagsBits::TransferDst bit");

		ICopyImageToBuffer(src, imageSubresource, dst, extent, srcOffset, dstOffset);
	}

	void RenderAPI::CopyImageToImage(
		const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& srcSubresource, 
		const ge::mem::Ref<Image>& dst, const ImageSubresourceLayers& dstSubresource, 
		glm::uvec3 extent, glm::uvec3 srcOffset, glm::uvec3 dstOffset
	) {
		GE_ASSERT(src->GetSpecRef().usageFlags.Has(ge::renderer::ImageUsageFlagsBits::TransferSrc), "src has ImageUsageFlagsBits::TransferSrc bit");
		GE_ASSERT(dst->GetSpecRef().usageFlags.Has(ge::renderer::ImageUsageFlagsBits::TransferDst), "dst has ImageUsageFlagsBits::TransferDst bit");

		ICopyImageToImage(src, srcSubresource, dst, dstSubresource, extent, srcOffset, dstOffset);
	}

	void RenderAPI::LoadDataToBuffer(const ge::mem::Ref<Buffer>& buffer, const void* data, uint64_t dataSize) {
		GE_ASSERT(buffer->GetSpecRef().usageFlags.Has(ge::renderer::BufferUsageFlagsBits::TransferDst), "buffer has BufferUsageFlagsBits::TransferDst bit");
		ILoadDataToBuffer(buffer, data, dataSize);
	}

	void RenderAPI::LoadDataToTexture2D(Texture2D& texture, const void* data, uint64_t dataSize) {
		GE_ASSERT(texture.GetImage()->GetSpecRef().usageFlags.Has(ge::renderer::ImageUsageFlagsBits::TransferDst), "texture image has ImageUsageFlagsBits::TransferDst bit");
		ILoadDataToTexture2D(texture, data, dataSize);
	}
}