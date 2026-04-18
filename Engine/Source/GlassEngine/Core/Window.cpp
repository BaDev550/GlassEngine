#include "gepch.h"
#include "Window.h"
#include <glfw/glfw3.h>

namespace ge {
	static bool s_glfwInitialized = false;
	Window::Window(const WindowSpecification& specs) : _specs(specs) {
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
		_swapchain = renderer::Swapchain::Create({
				.extent = {_specs.width, _specs.height},
				.depthFormat = renderer::ImageFormat::D32,
				.vsync = true,
				.srgb = true,
			}, *_renderContext);	

		Console::Get().AddCommand("window", "setTitle", [this](const GEVector<std::string>& args) { glfwSetWindowTitle(_handle, Console::GetMessageFromArgs(args).c_str()); }, "setTitle <title>");
	}

	Window::~Window() {
		glfwDestroyWindow(_handle);
		glfwTerminate();
	}

	uint32_t Window::GetImageIndex()
	{
		return _imageIndex;
	}

	void Window::PollEvents() const {
		glfwPollEvents();
	}

	void Window::SetIcon(std::vector<uint8_t> data, uint32_t width, uint32_t height)
	{
		GLFWimage images[1];
		images[0].pixels = data.data();
		images[0].width = width;
		images[0].height = height;
		glfwSetWindowIcon(_handle, 1, images);
	}

	bool Window::Swapbuffers()
	{
		return _swapchain->Swapbuffers(&_imageIndex);
	}

	bool Window::ShoudClose() const {
		return glfwWindowShouldClose(_handle);
	}
}