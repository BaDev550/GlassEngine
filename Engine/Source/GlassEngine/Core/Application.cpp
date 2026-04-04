#include "Application.h"
#include <stdexcept>
#include <iostream>

namespace ge {
	Application* Application::_instance = nullptr;
	Application::Application(const ApplicationCreateInfo& createInfo) {
		if (_instance)
			throw std::runtime_error("Application already exists!");
		_instance = this;

		Logger::Init();
		GE_CORE_INFO("Application created with title: {}, width: {}, height: ", createInfo.title, createInfo.width, createInfo.height);
	}

	Application::~Application() {
		Logger::Destroy();
	}

	void Application::Run() {
		while (!_forceClose) {

			for (auto& layer : _layerStack)
				layer->OnUpdate(0.0f);
		}
	}
}