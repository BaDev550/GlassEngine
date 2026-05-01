#include "gepch.h"
#include "Window.h"
#include <GLFW/glfw3.h>

namespace ge {
	static bool s_glfwInitialized = false;
	Window::Window(const WindowSpecification& specs) : _specs(specs) {
		if (!s_glfwInitialized) {
			GE_ASSERT(glfwInit(), "Failed to initialize GLFW");
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		{
			GE_PROFILE_SCOPE("CreatingWindow");
			_handle = glfwCreateWindow(_specs.width, _specs.height, _specs.title.c_str(), NULL, NULL);
			glfwMakeContextCurrent(_handle);
			glfwSetWindowUserPointer(_handle, &_specs);
			glfwSetFramebufferSizeCallback(_handle, FramebufferResizeCallback);
			GE_CORE_INFO("Window created!");
		}

		_renderContext = renderer::RenderContext::Create(_handle);
		_renderContext->Init();
		_swapchain = renderer::Swapchain::Create({
				.extent = {_specs.width, _specs.height},
				.depthFormat = renderer::ImageFormat::D32,
				.vsync = true,
				.srgb = true,
			}, *_renderContext);	

		GE_ADD_CONSOLE_COMMAND("window", "setTitle", [this](const GEVector<GEString>& args) { glfwSetWindowTitle(_handle, Console::GetMessageFromArgs(args).c_str()); }, "setTitle <title>");
	}

	Window::~Window() {
		_swapchain = nullptr;
		_renderContext = nullptr;
		glfwDestroyWindow(_handle);
		glfwTerminate();
	}

	uint32_t Window::GetImageIndex() const
	{
		return _imageIndex;
	}

	void Window::SetCursor(bool enabled)
	{
		glfwSetInputMode(_handle, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
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

	bool Window::HasResized() const
	{
		return _specs.resized;
	}

	void Window::ResetResizeFlag()
	{
		_specs.resized = false;
	}

	void Window::SetIcon(const GEString& iconPath)
	{
		auto iconAsset = ge::SourceTextureSerializer::ImportTextureFromFile(ImportAssetData(), iconPath.ToPath()).Cast<renderer::Texture2D>();
 		SetIcon(iconAsset->GetData(), iconAsset->GetWidth(), iconAsset->GetHeight());
	}

	void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		WindowSpecification* spec = reinterpret_cast<WindowSpecification*>(glfwGetWindowUserPointer(window));
		spec->resized = true;
		spec->width = width;
		spec->height = height;
	}
}