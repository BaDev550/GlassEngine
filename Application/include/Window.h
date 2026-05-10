#pragma once
#include <Core.h>

struct GLFWwindow;
namespace ge {
	struct WindowSpec {
		GEString title;
		uint32_t width;
		uint32_t height;
		bool resized = false;
	};

	class Window {
		Window(const WindowSpec& specs);
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window();

		GLFWwindow* GetHandle() const { return _handle; }
		uint32_t GetWidth() const { return _specs.width; }
		uint32_t GetHeight() const { return _specs.height; }
		void SetCursor(bool enabled);
		void PollEvents() const;
		void SetIcon(GEVector<uint8_t> data, uint32_t width, uint32_t height);
		bool ShoudClose() const;
		bool HasResized() const;
		void ResetResizeFlag();
	private:
		GLFWwindow* _handle;
		WindowSpec _specs;
	};
}