#include "gepch.h"
#include "Application.h"
#include <stdexcept>
#include <iostream>

namespace ge {
	Application* Application::_instance = nullptr;
	Application::Application(const ApplicationSpecification& specs) : _specs(specs) {
		if (_instance)
			throw std::runtime_error("Application already exists!");
		_instance = this;

		Logger::Init();
		profiler::Profiler::Init();
		GE_CORE_INFO("Application created with title: {}, width: {}, height: ", _specs.title, _specs.width, _specs.height);
		_window = mem::CreateScope<Window>(WindowSpecification({ _specs.title, _specs.width, _specs.height }));
	}

	Application::~Application() {
		Logger::Destroy();
		profiler::Profiler::Destroy();
	}

	void Application::Run() {
		while (!_window->ShoudClose() && !_forceClose) {
			_window->PollEvents();

			for (auto& layer : _layerStack)
				layer->OnUpdate(0.0f);
		}
		_window->GetRenderContext().Wait();
		profiler::utils::WriteProfileToFile("GEAR_ProfilerOutput.json");
	}
}