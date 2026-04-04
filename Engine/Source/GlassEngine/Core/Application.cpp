#include "Application.h"
#include <stdexcept>
#include <iostream>

namespace ge {
	Application* Application::_instance = nullptr;
	Application::Application(const ApplicationCreateInfo& createInfo) {
		if (_instance)
			throw std::runtime_error("Application already exists!");
		_instance = this;
		std::cout << "Application created with title: " << createInfo.title << ", width: " << createInfo.width << ", height: " << createInfo.height << std::endl;
	}

	void Application::Run() {
		while (!_forceClose) {

		}
	}
}