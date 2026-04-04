#pragma once
#include <cstdint>
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Renderer/RenderContext.h"

struct GLFWwindow;
namespace ge {
	struct WindowSpecification {
		GEString title;
		uint32_t width;
		uint32_t height; // TODO (0x): add func callback for events
	};

	class Window {
	public:
		Window(const WindowSpecification& specs);
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window();

		uint32_t GetWidth() const { return _specs.width; }
		uint32_t GetHeight() const { return _specs.height; }
		void PollEvents() const;
		bool ShoudClose() const;
	private:
		WindowSpecification _specs;
		GLFWwindow* _handle;

		mem::Scope<renderer::RenderContext> _renderContext = nullptr;
	};
}