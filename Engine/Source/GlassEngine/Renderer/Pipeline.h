#pragma once
#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Shader.h"
#include "GlassEngine/Renderer/Framebuffer.h"
#include "GlassEngine/Renderer/Types.h"

namespace ge::renderer {
	struct VertexAttribute {
		VertexFormat format;
		uint8_t offset;
		uint8_t location;
		uint8_t binding;
	};

	struct VertexBinding {
		uint16_t stride;
		uint8_t binding;
		VertexInputRate inputRate;
	};

	struct StencilOpState {
		StencilOp depthFailOp;
		StencilOp failOp;
		StencilOp passOp;
		CompareOp compareOp;
	};

	struct InputAssemblySpec {
		GEVector<VertexAttribute> vertexAttributes;
		GEVector<VertexBinding> vertexBindings;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;
	};

	struct ResterizerSpec {
		PolygonMode polygonMode;
		CullMode cullMode;
		ImageSampleCount sampleCount;
	};

	struct DepthStencilSpec {
		CompareOp depthTestCompareOp;
		StencilOpState stencilFrontOp;
		StencilOpState stencilBackOp;
		uint8_t stencilWriteMask;
		uint8_t stencilCompareMask;
		bool depthTestEnable;
		bool depthWriteEnable;
		bool stencilTestEnable;
	};

	// TODO (dnm): more detailed spec
	struct BlendSpec {
		bool blendEnabled;
	};

	struct PipelineSpec {
		InputAssemblySpec inputAssemblySpec;
		ResterizerSpec resterizerSpec;
		DepthStencilSpec depthStencilSpec;
		BlendSpec blendSpec;
		ge::mem::Ref<Shader> shader = nullptr;
		ge::mem::Ref<Framebuffer> targetFramebuffer = nullptr;
	};

	class Pipeline : public RenderObject {
	public:
		Pipeline(const PipelineSpec& spec) : _specs(spec) {}
		virtual ~Pipeline() = default;
		PipelineSpec GetSpecification() const { return _specs; }
		ge::mem::Ref<Shader>& GetShader() { return _specs.shader; }

		static ge::mem::Ref<Pipeline> Create(const PipelineSpec& specs);
	protected:
		PipelineSpec _specs;
	};
}