#pragma once
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_uint2.hpp>
#include <glm/ext/vector_uint3.hpp>
#include <glm/ext/vector_uint4.hpp>
#include <span>
#include <iostream>
#include <string_view>
#include "GlassEngine/Memory/Memory.h"

#include "GlassEngine/Memory/Ref.h"
#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Texture.h"
#include "GlassEngine/Renderer/Model.h"
#include "GlassEngine/Renderer/Types.h"
#include "GlassEngine/Renderer/Pipeline.h"

namespace ge::renderer {
	struct RenderStats {
		uint32_t drawCalls;

		void Reset() { drawCalls = 0; }
	};

	enum class GraphicsAPI {
		Vulkan,
		OpenGL,
		DirectX11,
		DirectX12
	};

	struct Attachment {
		ge::mem::Ref<Image> image;
		ImageSubresource subresource;
		ClearValue clearValue;
		AttachmentLoadOp loadOp;
		AttachmentStoreOp storeOp;

		bool isSwapchainImage;
	};

	struct BeginRenderPassSpec {
		std::span<const Attachment> colorAttachments;
		Attachment depthStencilAttachment;
		glm::uvec2 extent;
		ImageSampleCount sampleCount;
	};

	struct ImageSubresourceLayers {
		uint32_t mipLevel;
		uint32_t baseArrayLayer;
		uint32_t layerCount;
	};

	class RenderAPI : public RenderObject {
	public:
		RenderAPI() = default;
		virtual ~RenderAPI() = default;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void Draw(ge::mem::Ref<Pipeline>& pipeline, uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
		virtual void DrawIndexed(ge::mem::Ref<Pipeline>& pipeline, uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex = 0, uint32_t firstInstance = 0, int32_t vertexOffset = 0) = 0;
		virtual void DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex = 0, ge::mem::Ref<MaterialTable> materialTable = nullptr, const glm::mat4& transform = glm::mat4(1.0f)) = 0;

		virtual void PushConstant(const void *ptr, uint16_t size, uint16_t offset) = 0;

		virtual void BeginCopyPass() = 0;
		virtual void EndCopyPass() = 0;
		virtual void BeginRenderPass(const BeginRenderPassSpec&) = 0;
		virtual void EndRenderPass() = 0;

		void CopyBufferToBuffer(const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0);
		void CopyBufferToImage(const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Image>& dst, const ImageSubresourceLayers& imageSubresource, glm::uvec3 extent, uint32_t srcOffset = 0, glm::uvec3 dstOffset = {0,0,0});
		void CopyImageToBuffer(const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& imageSubresource, const ge::mem::Ref<Buffer>& dst, glm::uvec3 extent, glm::uvec3 srcOffset = {0,0,0}, uint32_t dstOffset = 0);
		void CopyImageToImage(const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& srcSubresource, const ge::mem::Ref<Image>& dst, const ImageSubresourceLayers& dstSubresource, glm::uvec3 extent, glm::uvec3 srcOffset = {0,0,0}, glm::uvec3 dstOffset = {0,0,0});

		void LoadDataToBuffer(const ge::mem::Ref<Buffer>& buffer, const void* data, uint64_t dataSize);
		void LoadDataToTexture2D(Texture2D& texture, const void* data, uint64_t dataSize);

		static inline RenderStats GetRenderStats() { return _renderStats; }
		static inline GraphicsAPI GetAPI() { return _graphicsAPI; }
		static void SetAPI(GraphicsAPI api) { _graphicsAPI = api; }
		static ge::mem::Ref<RenderAPI> Create();

		virtual void BeginDebugLabel(std::string_view label) = 0;
		virtual void EndDebugLabel() = 0;
		virtual void SetDebugName(GEString name) const noexcept final {}
	protected:
		virtual void ICopyBufferToBuffer(
			const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Buffer>& dst, 
			uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0
		) = 0;
		virtual void ICopyBufferToImage(
			const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Image>& dst, 
			const ImageSubresourceLayers& imageSubresource, glm::uvec3 extent, uint32_t srcOffset = 0, glm::uvec3 dstOffset = {0,0,0}
		) = 0;
		virtual void ICopyImageToBuffer(
			const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& imageSubresource, 
			const ge::mem::Ref<Buffer>& dst, glm::uvec3 extent, glm::uvec3 srcOffset = {0,0,0}, uint32_t dstOffset = 0
		) = 0;
		virtual void ICopyImageToImage(
			const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& srcSubresource, 
			const ge::mem::Ref<Image>& dst, const ImageSubresourceLayers& dstSubresource, 
			glm::uvec3 extent, glm::uvec3 srcOffset = {0,0,0}, glm::uvec3 dstOffset = {0,0,0}
		) = 0;

		virtual void ILoadDataToBuffer(const ge::mem::Ref<Buffer>& buffer, const void* data, uint64_t dataSize) = 0;
		virtual void ILoadDataToTexture2D(Texture2D& texture, const void* data, uint64_t dataSize) = 0;

		static RenderStats _renderStats;
		static GraphicsAPI _graphicsAPI;
	};
}
