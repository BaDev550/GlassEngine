#pragma once
#include <Core.h>
#include <Logger.h>
#include <Memory/Memory.h>

#include "Window.h"

namespace ge {
	struct ApplicationCommandLineArgs {
		int count = 0;
		char** args = nullptr;
		const char* operator[](int index) const { return args[index]; }
	};

	struct ApplicationSpecification {
		GEString title = "Glass Application";
		uint32_t width = 1280;
		uint32_t height = 720;
		ApplicationCommandLineArgs commandLineArgs;
	};

	class GE_API Application {
	public:
		Application(const ApplicationSpecification& specs);
		virtual ~Application() = default;

		virtual void OnStart() = 0;

		void Update(float deltaTime);
		void Close();
		bool Running() const { return !_forceClose; }

		mem::Ref<Sink>& GetGlobalSink() { return _globalSink; }
		Window& GetWindow() { return *_window; }
		ApplicationSpecification GetSpecs() const { return _specs; }
	private:
		ApplicationSpecification _specs;
		mem::Scope<Window> _window;
		mem::Ref<Sink> _globalSink;

		bool _forceClose = false;
	};
	Application* CreateApplication(const ApplicationCommandLineArgs& args);
}