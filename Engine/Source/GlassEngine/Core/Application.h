#pragma once
#include <cstdint>
#include <string>

#include "GlassEngine/Layers/LayerStack.h"
#include "GlassEngine/Utilities/Logger.h"
#include "GlassEngine/Core/Window.h"
#include "GlassEngine/Core/Core.h"

#include "GlassEngine/Asset/AssetManager.h"

namespace ge {
	struct ApplicationSpecification {
		GEString title = "Glass Engine";
		uint32_t width = 1280;
		uint32_t height = 720;
	};

	class Application {
	public:
		Application(const ApplicationSpecification& specs);
		virtual ~Application();

		void Run();
		void Close() { _forceClose = true; }

		Window& GetWindow() { return *_window; }
		EditorAssetManager& GetAssetManager() { return *_assetManager; }
		static Application* Get() { return _instance; }
		ApplicationSpecification GetSpecs() const { return _specs; }
	protected:
		void PushLayer(Layer* layer) { _layerStack.PushLayer(layer); }
		void PushOverlay(Layer* overlay) { _layerStack.PushOverlay(overlay); }
	private:
		static Application* _instance;
		ApplicationSpecification _specs;
		mem::Scope<Window> _window;
		LayerStack _layerStack;

		mem::Scope<EditorAssetManager> _assetManager; // Temp variable in application it needs to be moved in project class

		bool _forceClose = false;
	};

	Application* CreateApplication(const ApplicationSpecification& createInfo);
}