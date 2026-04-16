#pragma once
#include "GlassEngine/Core/Memory.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pipeline.h"

namespace ge::renderer {
	class RenderPass : public ge::mem::RefCounted {
	public:
		virtual void Begin(uint32_t layer = 0) = 0;
		virtual void End(uint32_t layer = 0) = 0;

		void SetInput(std::string_view name, const ge::mem::Ref<Buffer>& buffer);
		void SetInput(std::string_view name, const ge::mem::Ref<Sampler>& sampler);
		void SetInput(std::string_view name, const ge::mem::Ref<Texture2D>& texture);

		static ge::mem::Ref<RenderPass> Create(const ge::mem::Ref<Pipeline>& pipeline);
	protected:
		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Buffer>& buffer) = 0;
		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Sampler>& sampler) = 0;
		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Texture2D>& texture) = 0;
		ge::mem::Ref<Pipeline> _pipeline;

		const ShaderResource* GetShaderResource(std::string_view name);
	};
}