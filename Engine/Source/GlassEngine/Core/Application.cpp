#include "gepch.h"
#include "Application.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Texture.h"
#include "GlassEngine/Asset/AssetManager.h"
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
		_threadManager = mem::CreateScope<ThreadManager>(1);
		_editorAssetManager = (_specs.mode == ApplicationMode::Editor) ? mem::CreateScope<EditorAssetManager>() : nullptr;
		_runtimeAssetManager = (_specs.mode == ApplicationMode::Runtime) ? mem::CreateScope<RuntimeAssetManager>("assets.pak", "assets_manifest.bin") : nullptr;

		_window = mem::CreateScope<Window>(WindowSpecification({ _specs.title, _specs.width, _specs.height }));
		_window->SetIcon("Resources/icon-512.png");
		renderer::Renderer3D::Init();
		_imGuiLayer = ImGuiLayer::Create();

		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_ENGINE_CATAGORY, "close", [this](const GEVector<GEString>& args) { _forceClose = true; });
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_ENGINE_CATAGORY, "writeProfile", [](const GEVector<GEString>& args) { profile::utils::WriteEventsToFile(profile::Profiler::Get().GetEvents(), args[0].ToPath());}, "writeProfile <filePath>");
	}

	Application::~Application() {
#ifdef GE_APPLICATION_DUMP_LOG_ON_CLOSE
		GE_GLOBAL_SINK->Dump("Log_" + Time::GetCurrentLocalTime() + ".txt");
#endif
		delete _imGuiLayer;
		_imGuiLayer = nullptr;
		renderer::Renderer3D::Destroy();
		Logger::Destroy();
		Console::Destroy();
		profile::Profiler::Destroy();
	}

	void Application::Run() {
		while (!_window->ShoudClose() && !_forceClose) {
			_window->PollEvents();

			ge::renderer::Renderer3D::BeginFrame();
			for (auto& layer : _layerStack) {
				layer->OnUpdate(0.0f);

				_imGuiLayer->Begin();
				layer->OnImGuiRender();
				_imGuiLayer->End();
			}
			ge::renderer::Renderer3D::EndFrame();
		}
		_window->GetRenderContext().Wait();
	}
}