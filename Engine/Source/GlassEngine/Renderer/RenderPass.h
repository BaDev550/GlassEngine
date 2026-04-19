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

		void SetInput(std::string_view name, const ge::mem::Ref<Buffer>& buffer, uint16_t firstElement, uint16_t elementCount, uint16_t resourceIndex);
		void SetInput(std::string_view name, const ge::mem::Ref<Sampler>& sampler, uint16_t resourceIndex);
		void SetInput(std::string_view name, const ge::mem::Ref<Image>& image, ImageSubresource subresource, uint16_t resourceIndex);

		void SetInput(std::string_view name, const ge::mem::Ref<Texture>& texture, uint16_t resourceIndex);

		static ge::mem::Ref<RenderPass> Create(const ge::mem::Ref<Pipeline>& pipeline);
	protected:
		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Buffer>& buffer, uint16_t firstElement, uint16_t elementCount, uint16_t resourceIndex) = 0;
		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Sampler>& sampler, uint16_t resourceIndex) = 0;
		virtual void ISetInput(const ShaderResource& resource, const ge::mem::Ref<Image>& texture, ImageSubresource subresource, uint16_t resourceIndex) = 0;
		ge::mem::Ref<Pipeline> _pipeline;

		const ShaderResource* GetShaderResource(std::string_view name);
	};

	inline void RenderPass::SetInput(std::string_view name, const ge::mem::Ref<Texture>& texture, uint16_t resourceIndex) {
		SetInput(name, texture, resourceIndex);
	}
}