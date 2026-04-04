#pragma once
#include <cstdint>
#include <string>

namespace ge {
	struct ApplicationCreateInfo {
		std::string title = "Glass Engine";
		uint32_t width = 1280;
		uint32_t height = 720;
	};

	class Application {
	public:
		Application(const ApplicationCreateInfo& createInfo);
		virtual ~Application() = default;

		void Run();
		void Close() { _forceClose = true; }
	private:
		static Application* _instance;
		bool _forceClose = false;
	};

	Application* CreateApplication(const ApplicationCreateInfo& createInfo);
}