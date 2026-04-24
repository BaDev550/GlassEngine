#pragma once
#include "GlassEngine/Memory/Memory.h"
#include "Buffer.h"
#include "GlassEngine/Renderer/Framebuffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pipeline.h"

namespace ge::renderer {
	class RenderPass : public ge::mem::RefCounted {
	public:
		RenderPass(const ge::mem::Ref<Framebuffer>& framebuffer) : _framebuffer(framebuffer) {}
		
		virtual void Begin(uint32_t layer = 0) = 0;
		virtual void End(uint32_t layer = 0) = 0;

		static ge::mem::Ref<RenderPass> Create(const ge::mem::Ref<Framebuffer>& framebuffer);
	protected:
		ge::mem::Ref<Framebuffer> _framebuffer;
	};
}