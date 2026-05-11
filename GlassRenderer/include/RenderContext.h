#pragma once
#include <Graphics/Types.h>
#include <Memory/Memory.h>

#include <Engine.h>
#include <Logger.h>

#include <Graphics/Interfaces/IRenderContext.h>

namespace ge::graphics {
	class IRenderContext_Impl : public IRenderContext {
	public:
		virtual ~IRenderContext_Impl() = default;
		static ge::mem::Scope<IRenderContext_Impl> Create(GLFWwindow* window);
	};
}