#pragma once
#include <cstdint>
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Renderer/RenderContext.h"
#include "GlassEngine/Renderer/Swapchain.h"

struct GLFWwindow;
namespace ge {
	struct WindowSpecification {
		GEString title;
		uint32_t width;
		uint32_t height;
	};

	class Window {
	public:
		Window(const WindowSpecification& specs);
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window();

		GLFWwindow* GetHandle() const { return _handle; }
		uint32_t GetWidth() const { return _specs.width; }
		uint32_t GetHeight() const { return _specs.height; }
		uint32_t GetImageIndex() const;
		void PollEvents() const;
		void SetIcon(std::vector<uint8_t> data, uint32_t width, uint32_t height);
		void SetIcon(const GEString& iconPath);
		bool Swapbuffers();
		bool ShoudClose() const;

		renderer::RenderContext& GetRenderContext() { return *_renderContext; }
		renderer::Swapchain& GetSwapchain() { return *_swapchain; }
		const renderer::Swapchain& GetSwapchain() const { return *_swapchain; }
	private:
		WindowSpecification _specs;
		GLFWwindow* _handle;
		uint32_t _imageIndex = 0;

		mem::Scope<renderer::RenderContext> _renderContext = nullptr;
		mem::Scope<renderer::Swapchain> _swapchain = nullptr;
	};
}