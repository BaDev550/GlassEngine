#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Core/Application.h"
#include "RenderContext.h"

namespace ge::renderer {
	class RenderObject : public virtual ge::mem::RefCounted {
	public:
		explicit RenderObject() noexcept : _renderContext(Application::Get()->GetWindow().GetRenderContext()) {};
	protected:
		RenderContext& _renderContext;
	};
}