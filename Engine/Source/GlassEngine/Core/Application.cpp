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

		Console::Init();
		Logger::Init();
		_threadManager = mem::CreateScope<ThreadManager>(1);
		_editorAssetManager = mem::CreateScope<EditorAssetManager>();
		_runtimeAssetManager = mem::CreateScope<RuntimeAssetManager>("Assets.pak");

		_window = mem::CreateScope<Window>(WindowSpecification({ _specs.title, _specs.width, _specs.height }));
		_window->SetIcon("Resouces/icon-512.png");
		renderer::Renderer3D::Init();
		_imGuiLayer = ImGuiLayer::Create();

		Console::Get().AddCommand("engine", "close", [this](const GEVector<std::string>& args) { _forceClose = true; });
		Console::Get().AddCommand("engine", "help", [](const GEVector<std::string>& args) {
			GE_CORE_INFO("Available commands:");
			for (const auto& list : ge::Console::Get().GetCommandLists()) {
				for (const auto& cmd : list.commands) {
					std::string usageInfo = cmd.usage.empty() ? "" : (" - " + cmd.usage);
					GE_CORE_INFO("- {}.{}{}", list.name, cmd.name, usageInfo);
				}
			}});
	}

	Application::~Application() {
		delete _imGuiLayer;
		_imGuiLayer = nullptr;
		renderer::Renderer3D::Destroy();
		Logger::Destroy();
		Console::Destroy();
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