#pragma once
#include <cstdint>

struct GLFWwindow;
namespace ge {
	struct WindowCreateInfo {
		uint32_t width;
		uint32_t height; // TODO (0x): add more variables for controll
	};

	class Window {
	public:
		Window(const WindowCreateInfo& createInfo);
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window();

		bool ShoudClose() const;
	private:
		GLFWwindow* _handle;
	};
}