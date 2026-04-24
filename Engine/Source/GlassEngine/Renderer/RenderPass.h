#pragma once
#include "GlassEngine/Core/String.h"
#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Renderer/Framebuffer.h"

namespace ge::renderer {
	class RenderPass : public ge::mem::RefCounted {
	public:
		RenderPass(const ge::mem::Ref<Framebuffer>& framebuffer, std::string_view debugName) : _framebuffer(framebuffer), _debugName(debugName) {}
		
		virtual void Begin(uint32_t layer = 0) = 0;
		virtual void End(uint32_t layer = 0) = 0;

		static ge::mem::Ref<RenderPass> Create(const ge::mem::Ref<Framebuffer>& framebuffer, std::string_view debugName = "");
	protected:
		ge::mem::Ref<Framebuffer> _framebuffer;
		GEString _debugName;
	};
}