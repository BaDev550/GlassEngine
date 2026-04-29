#pragma once

#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Core/Engine.h"
#include "RenderContext.h"

namespace ge::renderer {
	class RenderObject : public virtual ge::mem::RefCounted {
	public:
		explicit RenderObject() noexcept : _renderContext(Engine::Get().GetApplicationWindow().GetRenderContext()) {};

		//TODO (0x): it should probably be a wide string
		virtual void SetDebugName(GEString name) const noexcept = 0;
	protected:
		RenderContext& _renderContext;
	};
}