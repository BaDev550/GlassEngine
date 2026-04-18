#pragma once
#include <cstdint>
#include <string>

#include "GlassEngine/Layers/LayerStack.h"
#include "GlassEngine/Layers/ImGuiLayer.h"
#include "GlassEngine/Utilities/Logger.h"
#include "GlassEngine/Thread/ThreadManager.h"
#include "GlassEngine/Core/Window.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Console.h"

#include "GlassEngine/Asset/AssetManager.h"

namespace ge {
	enum class ApplicationMode {
		Editor,
		Runtime
	};

	struct ApplicationSpecification {
		GEString title = "Glass Engine";
		ApplicationMode mode = ApplicationMode::Editor;
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
		ThreadManager& GetThreadManager() { return *_threadManager; }
		LayerStack& GetLayerStack() { return _layerStack; }
		EditorAssetManager& GetEditorAssetManager() { return *_editorAssetManager; }
		RuntimeAssetManager& GetRuntimeAssetManager() { return *_runtimeAssetManager; }
		AssetManager& GetDefaultAssetManager() { return *_defaultAssetManager; }
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
		ImGuiLayer* _imGuiLayer;

		AssetManager* _defaultAssetManager;
		mem::Scope<EditorAssetManager> _editorAssetManager;
		mem::Scope<RuntimeAssetManager> _runtimeAssetManager;
		mem::Scope<ThreadManager> _threadManager;

		bool _forceClose = false;
	};

	Application* CreateApplication(const ApplicationSpecification& createInfo);
}