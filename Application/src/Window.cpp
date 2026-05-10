#include "Window.h"
#include <GLFW/glfw3.h>

namespace ge {
	static bool s_glfwInitialized = false;
	Window::Window(const WindowSpec& specs) : _specs(specs) {
		if (!s_glfwInitialized) {
			GE_ASSERT(glfwInit(), "Failed to initialize GLFW");
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		{
			_handle = glfwCreateWindow(_specs.width, _specs.height, _specs.title.c_str(), NULL, NULL);
			glfwMakeContextCurrent(_handle);
			glfwSetWindowUserPointer(_handle, &_specs);
			//GE_LOG(LL_Info, "Window created!");
		}
	}

	Window::~Window() {
		glfwDestroyWindow(_handle);
		glfwTerminate();
	}

	void Window::SetCursor(bool enabled)
	{
		glfwSetInputMode(_handle, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}

	void Window::PollEvents() const {
		glfwPollEvents();
	}

	void Window::SetIcon(GEVector<uint8_t> data, uint32_t width, uint32_t height)
	{
		GLFWimage images[1];
		images[0].pixels = data.data();
		images[0].width = width;
		images[0].height = height;
		glfwSetWindowIcon(_handle, 1, images);
	}

	bool Window::ShoudClose() const {
		return glfwWindowShouldClose(_handle);
	}

	bool Window::HasResized() const
	{
		return _specs.resized;
	}

	void Window::ResetResizeFlag()
	{
		_specs.resized = false;
	}
}