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
		static Application* Get() { return _instance; }

		Application(const ApplicationSpecification& specs);
		virtual ~Application();

		void Run();
		void Close() { _forceClose = true; }

		Window& GetWindow() { return *_window; }
		LayerStack& GetLayerStack() { return _layerStack; }
		ThreadManager& GetThreadManager() { return *_threadManager; }
		EditorAssetManager& GetEditorAssetManager() { return *_editorAssetManager; }
		RuntimeAssetManager& GetRuntimeAssetManager() { return *_runtimeAssetManager; }
		ApplicationSpecification GetSpecs() const { return _specs; }
	protected:
		void PushLayer(Layer* layer) { _layerStack.PushLayer(layer); }
		void PushOverlay(Layer* overlay) { _layerStack.PushOverlay(overlay); }
	private:
		static Application* _instance;
		ApplicationSpecification _specs;
		LayerStack _layerStack;
		ImGuiLayer* _imGuiLayer;

		mem::Scope<Window> _window;
		mem::Scope<ThreadManager> _threadManager;
		mem::Scope<EditorAssetManager> _editorAssetManager;
		mem::Scope<RuntimeAssetManager> _runtimeAssetManager;

		bool _forceClose = false;
	};

	Application* CreateApplication(const ApplicationSpecification& createInfo);
}