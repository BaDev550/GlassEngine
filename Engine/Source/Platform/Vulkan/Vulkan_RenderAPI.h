#pragma once
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Memory/Ref.h"
#include "GlassEngine/Renderer/RenderAPI.h"
#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Texture.h"
#include "Platform/Vulkan/Vulkan_Swapchain.h"
#include "Vulkan_RenderContext.h"
#include <unordered_set>

namespace ge::renderer {
	class Vulkan_RenderAPI final : public RenderAPI {
	public:
		Vulkan_RenderAPI();
		~Vulkan_RenderAPI();

		virtual void Draw(ge::mem::Ref<Pipeline>& pipeline, uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
		virtual void DrawIndexed(ge::mem::Ref<Pipeline>& pipeline, uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex = 0, uint32_t firstInstance = 0, int32_t vertexOffset = 0) override;
		virtual void DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex = 0, ge::mem::Ref<MaterialTable> materialTable = nullptr, const glm::mat4& transform = glm::mat4(1.0f)) override;

		virtual void PushConstant(const void *ptr, uint16_t size, uint16_t offset) override;

		virtual void BeginCopyPass() override;
		virtual void EndCopyPass() override;
		virtual void BeginRenderPass(const BeginRenderPassSpec&) override;
		virtual void EndRenderPass() override;

		virtual void ICopyBufferToBuffer(
			const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Buffer>& dst, 
			uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0
		) override;
		virtual void ICopyBufferToImage(
			const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Image>& dst, 
			const ImageSubresourceLayers& imageSubresource, glm::uvec3 extent, uint32_t srcOffset = 0, glm::uvec3 dstOffset = {0,0,0}
		) override;
		virtual void ICopyImageToBuffer(
			const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& imageSubresource, 
			const ge::mem::Ref<Buffer>& dst, glm::uvec3 extent, glm::uvec3 srcOffset = {0,0,0}, uint32_t dstOffset = 0
		) override;
		virtual void ICopyImageToImage(
			const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& srcSubresource, 
			const ge::mem::Ref<Image>& dst, const ImageSubresourceLayers& dstSubresource, 
			glm::uvec3 extent, glm::uvec3 srcOffset = {0,0,0}, glm::uvec3 dstOffset = {0,0,0}
		) override;

		virtual ImTextureID GetImGuiTexture(ge::mem::Ref<Image>& image, ImageSubresource subresource, ge::mem::Ref<Sampler> sampler = {}) override;

		virtual void ILoadDataToBuffer(const ge::mem::Ref<Buffer>& buffer, const void* data, uint64_t dataSize) override;
		virtual void ILoadDataToTexture2D(Texture2D& texture, const void* data, uint64_t dataSize) override;

		virtual void BeginDebugLabel(std::string_view label) override;
		virtual void EndDebugLabel() override;

		VkCommandBuffer GetCurrentCommandBuffer();
		auto GetCurrentInFlightFence() const noexcept { return _frames[Renderer3D::GetFrameIndex()].inFlightFence; }
	private:
		void TrackObject(ge::mem::Ref<RenderObject> object) {
			_frames[Renderer3D::GetFrameIndex()].renderObjects.push_back(object);
		}

		auto &GetCurrentFrame() const { return _frames[Renderer3D::GetFrameIndex()]; }
		auto &GetCurrentFrame() { return _frames[Renderer3D::GetFrameIndex()]; }

		void BeginCommandBuffer();
		void EndCommandBuffer();

		struct FrameContext {
			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;
			VkFence inFlightFence;
			bool isRecordState = false;
			bool swapchainImageUsed = false;
			// for life time
			GEVector<ge::mem::Ref<RenderObject>, ge::mem::RendererAllocTag> renderObjects;
			GEVector<ge::mem::Scope<Vulkan_Buffer>> stagingBuffers;
		} _frames[Renderer3D::MaxFramesInFlight];

		VkImageMemoryBarrier2 GetMemoryBarrier(Vulkan_Image& image, const VkImageLayout newImageLayout, bool loadOpIsLoad = false);

		// translates image layout to Vulkan_OptimalImageLayout() when called End*Pass() func
		std::unordered_set<Vulkan_Image*> _barriersForImages; // TODO (0x): better name
		std::unordered_set<Vulkan_Buffer*> _barriersForBuffers; // TODO (0x): better name
		
		void Barrier();
		friend Vulkan_Swapchain;
	};
}