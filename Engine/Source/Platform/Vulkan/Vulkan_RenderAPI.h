#pragma once
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Renderer/RenderAPI.h"
#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "Vulkan_RenderContext.h"
#include <unordered_set>

namespace ge::renderer {
	class Vulkan_RenderAPI final : public RenderAPI {
	public:
		Vulkan_RenderAPI();
		~Vulkan_RenderAPI();
		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex = 0, uint32_t firstInstance = 0, int32_t vertexOffset = 0) override;
		virtual void DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex = 0, ge::mem::Ref<MaterialTable> materialTable = nullptr, const glm::mat4& transform = glm::mat4(1.0f)) override;

		virtual void PushConstant(const void *ptr, uint16_t size, uint16_t offset) override;

		virtual void BeginCopyPass() override;
		virtual void EndCopyPass() override;
		virtual void BeginRenderPass(const BeginRenderPassSpec&) override;
		virtual void EndRenderPass() override;

		virtual void LoadDataToTexture2D(Texture2D& texture, void* data, uint64_t dataSize) override;

		virtual void SetBeginDebugLabel(std::string_view label) override;
		virtual void SetEndDebugLabel() override;

		VkCommandBuffer GetCurrentCommandBuffer();
	private:
		struct FrameContext {
			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;
			// for life time
			GEVector<RenderObject> renderObjects;
		} _frames[Renderer3D::MaxFramesInFlight]; // TEMP ig
		bool _frameStarted = false;

		// translates image layout to Vulkan_OptimalImageLayout() when called End*Pass() func
		std::unordered_set<Vulkan_Image*> _image_layout_transition_set;
		// TODO(dnm): maybe we track buffers
		void TransitionImageLayouts();
	};
}