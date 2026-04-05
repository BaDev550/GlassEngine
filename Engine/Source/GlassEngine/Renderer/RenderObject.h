#pragma once

#include "GlassEngine/Core/Memory.h"
#include "RenderContext.h"

namespace ge::renderer {
	class RenderObject : public ge::mem::RefCounted {
	public:
		explicit RenderObject() noexcept : _renderContext(Application::Get()->GetWindow().GetRenderContext()) {};
	protected:
		RenderContext& _renderContext;
	};
}