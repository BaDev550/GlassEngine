#pragma once
#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Shader.h"
#include "GlassEngine/Renderer/Framebuffer.h"

namespace ge::renderer {
	enum class CullMode {
		None = 0,
		Back,
		Front
	};

	enum class DepthMode {
		None = 0,
		Less,
		LessOrEqual
	};

	struct PipelineSpecification {
		ge::mem::Ref<Shader> shader = nullptr;
		ge::mem::Ref<Framebuffer> targetFramebuffer = nullptr;
		CullMode cullMode = CullMode::Back;
		DepthMode depthTest = DepthMode::Less;
	};

	class Pipeline : public RenderObject {
	public:
		virtual ~Pipeline() = default;
		PipelineSpecification GetSpecification() const { return _specs; }
		ge::mem::Ref<Shader>& GetShader() { return _specs.shader; }

		static ge::mem::Ref<Pipeline> Create(const PipelineSpecification& specs);
	protected:
		PipelineSpecification _specs;
	};
}