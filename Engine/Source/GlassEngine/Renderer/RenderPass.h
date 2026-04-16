#pragma once
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pipeline.h"

namespace ge::renderer {
	class RenderPass : public mem::RefCounted {
	public:
		virtual void Begin(uint32_t layer = 0) = 0;
		virtual void End(uint32_t layer = 0) = 0;

		virtual void SetInput(std::string_view name, const mem::Ref<Buffer>& buffer) = 0;
		virtual void SetInput(std::string_view name, const mem::Ref<Sampler>& sampler) = 0;
		virtual void SetInput(std::string_view name, const mem::Ref<Texture2D>& texture) = 0;

		static mem::Ref<RenderPass> Create(const mem::Ref<Pipeline>& pipeline);
	};
}