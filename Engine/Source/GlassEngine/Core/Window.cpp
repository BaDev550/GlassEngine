#include "gepch.h"
#include "Window.h"
#include <glfw/glfw3.h>

namespace ge {
	static bool s_glfwInitialized = false;
	Window::Window(const WindowSpecification& specs) : _specs(specs) {
		GE_PROFILE_SCOPE("GE_WindowCreate");
		if (!s_glfwInitialized) {
			GE_ASSERT(glfwInit(), "Failed to initialize GLFW");
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		_handle = glfwCreateWindow(_specs.width, _specs.height, _specs.title.c_str(), NULL, NULL);
		glfwMakeContextCurrent(_handle);
		glfwSetWindowUserPointer(_handle, &_specs);
		GE_CORE_INFO("Window created!");

		_renderContext = renderer::RenderContext::Create(_handle);
		_renderContext->Init();
	}

	Window::~Window() {
		glfwDestroyWindow(_handle);
		glfwTerminate();
	}

	void Window::PollEvents() const {
		glfwPollEvents();
	}

	bool Window::ShoudClose() const {
		return glfwWindowShouldClose(_handle);
	}
}