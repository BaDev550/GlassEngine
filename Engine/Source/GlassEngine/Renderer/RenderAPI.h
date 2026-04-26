#pragma once
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_uint2.hpp>
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
	};

	enum class GraphicsAPI {
		Vulkan,
		OpenGL,
		DirectX11,
		DirectX12
	};
	struct ColorClearColor {
		glm::vec4 color;
	};

	struct DepthStencilClearColor {
		float depth_clear;
		uint8_t stencil_clear;
	};

	struct ClearColor {
		ClearColor(float depth, uint8_t stencil) : depthClear(depth), stencilClear(stencil) {}
		ClearColor(glm::vec4 color) : colorClear(color) {}
		union {
			glm::vec4 colorClear;
			struct { float depthClear; uint8_t stencilClear; };
		};
	};

	struct Attachment {
		ge::mem::Ref<Image> image;
		ImageSubresource subresource;
		ClearColor clearColor;
		AttachmentLoadOp loadOp;
		AttachmentStoreOp storeOp;
	};

	struct BeginRenderPassSpec {
		std::span<const Attachment> color_attachments;
		Attachment depth_stencil;
		glm::uvec2 extent;
		ImageSampleCount sampleCount;
	};

	class RenderAPI : public RenderObject {
	public:
		RenderAPI() = default;
		virtual ~RenderAPI() = default;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex = 0, uint32_t firstInstance = 0, int32_t vertexOffset = 0) = 0;
		virtual void DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex = 0, ge::mem::Ref<MaterialTable> materialTable = nullptr, const glm::mat4& transform = glm::mat4(1.0f)) = 0;

		virtual void PushConstant(const void *ptr, uint16_t size, uint16_t offset) = 0;

		virtual void BeginCopyPass() = 0;
		virtual void EndCopyPass() = 0;
		virtual void BeginRenderPass(const BeginRenderPassSpec&) = 0;
		virtual void EndRenderPass() = 0;

		virtual void LoadDataToTexture2D(Texture2D& texture, void* data, uint64_t dataSize) = 0;

		static inline RenderStats GetRenderStats() { return _renderStats; }
		static inline GraphicsAPI GetAPI() { return _graphicsAPI; }
		static void SetAPI(GraphicsAPI api) { _graphicsAPI = api; }
		static ge::mem::Ref<RenderAPI> Create();

		virtual void SetBeginDebugLabel(std::string_view label) = 0;
		virtual void SetEndDebugLabel() = 0;
		virtual void SetDebugName(GEString name) const noexcept final {}
	protected:
		static RenderStats _renderStats;
		static GraphicsAPI _graphicsAPI;
	};
}
