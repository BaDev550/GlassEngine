#include "gepch.h"
#include "Engine.h"
#include "GlassEngine/Renderer/Renderer.h"
#include <nfd.hpp>

namespace ge {
	Engine* Engine::_instance = nullptr;
	Engine::Engine(const EngineSpecification& specs) : _specs(specs) {
		if (_instance)
			throw std::runtime_error("Engine already exists!");
		_instance = this;

		profile::Profiler::Init();
		Console::Init();

		WindowSpecification windowSpecs{};
		windowSpecs.width = specs.application->GetSpecs().width;
		windowSpecs.height = specs.application->GetSpecs().height;
		windowSpecs.title = specs.application->GetSpecs().title;
		_window = mem::CreateScope<Window>(windowSpecs);
		_inputManager = mem::CreateScope<InputManager>(_window->GetHandle());
		_imGuiLayer = ImGuiLayer::Create();

		renderer::Renderer3D::Init(); //def move this into engine
		_window->SetCursor(true);
		_window->SetIcon((GE_ENGINE_DIR.parent_path() / "Brand/icon-512.png").string());
		_specs.application->Init();
		NFD::Init();

		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_ENGINE_CATAGORY, "close", [this](const GEVector<GEString>& args) { GetApplication()->Close(); });
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_ENGINE_CATAGORY, "writeProfile", [](const GEVector<GEString>& args) { profile::utils::WriteEventsToFile(profile::Profiler::Get().GetEvents(), args[0].ToPath()); }, "writeProfile <filePath>");
	}

	Engine::~Engine() {}

	void Engine::Run() {
		Timer timer;
		while (!_window->ShoudClose() && GetApplication()->Running()) {
			float time = timer.Elapsed();
			_deltaTime = time - _lastTime;
			_lastTime = time;

			_window->PollEvents();
			_inputManager->Update();

			if (!renderer::Renderer3D::BeginFrame()) {
				continue;
			}

			GetApplication()->Update(_deltaTime);

			renderer::Renderer3D::Submit([this]() { _imGuiLayer->Begin(); GetApplication()->DrawImGui(); });
			renderer::Renderer3D::Submit([=]() { _imGuiLayer->End(); });

			renderer::Renderer3D::WaitAndRender();
			renderer::Renderer3D::EndFrame();
		}
		_window->GetRenderContext().Wait();
	}

	void Engine::Destroy() {
#ifdef GE_ENGINE_DUMP_LOG_ON_CLOSE
		GE_GLOBAL_SINK->Dump("Log_" + Time::GetCurrentLocalTime() + ".txt");
#endif
		_inputManager = nullptr;
		delete _imGuiLayer;
		_imGuiLayer = nullptr;
		_specs.application->Close();
		delete _specs.application;
		_specs.application = nullptr;
		NFD::Quit();
		AssetManager::Destroy();
		renderer::Renderer3D::Destroy();
		_window = nullptr;
		Console::Destroy();
		profile::Profiler::Destroy();
		MemoryProfiler::Destroy();
	}
}