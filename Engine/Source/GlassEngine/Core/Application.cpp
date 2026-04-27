#include "gepch.h"
#include "Application.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Texture.h"
#include "GlassEngine/Utilities/Timer.h"
#include <stdexcept>
#include <iostream>

namespace ge {
	Application* Application::_instance = nullptr;
	Application::Application(const ApplicationSpecification& specs) : _specs(specs) {
		if (_instance)
			throw std::runtime_error("Application already exists!");
		_instance = this;

		profile::Profiler::Init();
		Console::Init();
		Logger::Init();
		AssetManager::Init(nullptr);
		_threadManager = mem::CreateScope<ThreadManager>(1);
		_window = mem::CreateScope<Window>(WindowSpecification({ _specs.title, _specs.width, _specs.height }));
		renderer::Renderer3D::Init();

		_window->SetCursor(true);
		// TODO (0x): dont work with gpu compression
		// _window->SetIcon("Resources/icon-512.png");

		_imGuiLayer = ImGuiLayer::Create();
		Input::Init();

		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_ENGINE_CATAGORY, "close", [this](const GEVector<GEString>& args) { _forceClose = true; });
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_ENGINE_CATAGORY, "writeProfile", [](const GEVector<GEString>& args) { profile::utils::WriteEventsToFile(profile::Profiler::Get().GetEvents(), args[0].ToPath());}, "writeProfile <filePath>");
	}

	Application::~Application() {
#ifdef GE_APPLICATION_DUMP_LOG_ON_CLOSE
		GE_GLOBAL_SINK->Dump("Log_" + Time::GetCurrentLocalTime() + ".txt");
#endif
		_layerStack.Clear();
		AssetManager::Destroy();
		delete _imGuiLayer;
		_imGuiLayer = nullptr;
		renderer::Renderer3D::Destroy();
		_window = nullptr;
		Logger::Destroy();
		Console::Destroy();
		profile::Profiler::Destroy();
	}

	void Application::Run() {
		Timer timer;
		while (!_window->ShoudClose() && !_forceClose) {
			float time = timer.Elapsed();
			_deltaTime = time - _lastTime;
			_lastTime = time;

			_window->PollEvents();
			Input::Update();

			ge::renderer::Renderer3D::BeginFrame();
			for (auto& layer : _layerStack) {
				layer->OnUpdate(_deltaTime);

				_imGuiLayer->Begin();
				for (auto& [name, panel] : layer->GetPanelManager()) { panel->Draw(); }
				layer->OnImGuiRender();
				_imGuiLayer->End();
			}
			ge::renderer::Renderer3D::EndFrame();
		}
		_window->GetRenderContext().Wait();
	}
}