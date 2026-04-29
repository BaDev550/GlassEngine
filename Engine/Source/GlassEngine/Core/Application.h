#pragma once
#include <cstdint>
#include <string>

#include "GlassEngine/Layers/LayerStack.h"
#include "GlassEngine/Editor/PanelStack.h"
#include "GlassEngine/Layers/ImGuiLayer.h"
#include "GlassEngine/Utilities/Logger.h"
#include "GlassEngine/Core/Window.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Core/Input.h"
#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Thread/ThreadManager.h"
#include "GlassEngine/Utilities/Console.h"
#include "GlassEngine/Utilities/Profiler.h"

#include "GlassEngine/Asset/AssetManager.h"

namespace ge {
	enum class EngineIDE {
		VS22,
		VSC,
		VIM,
		CLion
	};

	enum class ApplicationMode {
		Editor,
		Runtime
	};

	struct ApplicationCommandLineArgs {
		int count = 0;
		char** args = nullptr;
		const char* operator[](int index) const { return args[index]; }
	};

	struct ApplicationSpecification {
		GEString title = "Glass Engine";
		ApplicationCommandLineArgs commandLineArgs;
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
		ApplicationSpecification GetSpecs() const { return _specs; }
	protected:
		void DrawImGui();
		void PushLayer(Layer* layer) { _layerStack.PushLayer(layer); }
		void PushOverlay(Layer* overlay) { _layerStack.PushOverlay(overlay); }
	private:
		static Application* _instance;
		ApplicationSpecification _specs;
		ImGuiLayer* _imGuiLayer;
		LayerStack _layerStack;

		mem::Scope<Window> _window;
		mem::Scope<ThreadManager> _threadManager;

		bool _forceClose = false;
		float _lastTime = 0.0f;
		float _deltaTime = 0.0f;
	};

	Application* CreateApplication(const ApplicationCommandLineArgs& createInfo);
}