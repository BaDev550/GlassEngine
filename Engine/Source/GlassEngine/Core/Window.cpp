#include "Window.h"
#include <glfw/glfw3.h>

namespace ge {
	static bool s_glfwInitialized = false;
	Window::Window(const WindowCreateInfo& createInfo) {
		if (!s_glfwInitialized) {
			
		}
	}
	Window::~Window()
	{
	}
	bool Window::ShoudClose() const
	{
		return false;
	}
}