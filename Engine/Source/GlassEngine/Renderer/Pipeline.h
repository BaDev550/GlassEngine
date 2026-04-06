#pragma once
#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Shader.h"

namespace ge::renderer {
	enum class CullMode {
		None = 0,
		Back,
		Front
	};

	struct PipelineSpecification {
		ge::mem::Ref<Shader> shader = nullptr;
		// TODO (badev): when framebuffer class is added to ptr into target framebuffer
		CullMode cullMode = CullMode::Back;
		bool depthTest = true;
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