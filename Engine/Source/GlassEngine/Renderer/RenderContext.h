#pragma once
#include "GlassEngine/Core/Memory.h"

struct GLFWwindow;

namespace ge::renderer {
	class RenderContext {
	public:
		virtual ~RenderContext() = default;
		virtual void Init() = 0;
		virtual void Wait() = 0;

		static ge::mem::Scope<RenderContext> Create(GLFWwindow* window);
	};
}