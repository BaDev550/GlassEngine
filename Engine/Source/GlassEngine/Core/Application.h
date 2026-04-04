#pragma once
#include <cstdint>
#include <string>

#include "GlassEngine/Layers/LayerStack.h"
#include "GlassEngine/Utilities/Logger.h"

namespace ge {
	struct ApplicationCreateInfo {
		std::string title = "Glass Engine";
		uint32_t width = 1280;
		uint32_t height = 720;
	};

	class Application {
	public:
		Application(const ApplicationCreateInfo& createInfo);
		virtual ~Application();

		void Run();
		void Close() { _forceClose = true; }
	protected:
		void PushLayer(Layer* layer) { _layerStack.PushLayer(layer); }
		void PushOverlay(Layer* overlay) { _layerStack.PushOverlay(overlay); }
	private:
		static Application* _instance;
		LayerStack _layerStack;

		bool _forceClose = false;
	};

	Application* CreateApplication(const ApplicationCreateInfo& createInfo);
}