#pragma once
#include <cstdint>
#include <string>

#include "GlassEngine/Layers/LayerStack.h"
#include "GlassEngine/Editor/PanelStack.h"
#include "GlassEngine/Layers/ImGuiLayer.h"
#include "GlassEngine/Core/Window.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Thread/ThreadManager.h"

namespace ge {
	struct ApplicationCommandLineArgs {
		int count = 0;
		char** args = nullptr;
		const char* operator[](int index) const { return args[index]; }
	};

	struct ApplicationSpecification {
		GEString title = "Glass Engine";
		uint32_t width = 1280;
		uint32_t height = 720;
		ApplicationCommandLineArgs commandLineArgs;
	};

	class Application {
	public:
		Application(const ApplicationSpecification& specs);
		virtual ~Application();

		void Update(float deltaTime);
		bool Running() const;
		void Close() { _forceClose = true; }

		LayerStack& GetLayerStack() { return _layerStack; }
		ThreadManager& GetThreadManager() { return *_threadManager; }
		ApplicationSpecification GetSpecs() const { return _specs; }
		void DrawImGui();

		virtual void Init() = 0;
	protected:
		void PushLayer(Layer* layer) { _layerStack.PushLayer(layer); }
		void PushOverlay(Layer* overlay) { _layerStack.PushOverlay(overlay); }
	private:
		ApplicationSpecification _specs;
		LayerStack _layerStack;

		mem::Scope<ThreadManager> _threadManager;

		bool _forceClose = false;
	};

	Application* CreateApplication(const ApplicationCommandLineArgs& createInfo);
}