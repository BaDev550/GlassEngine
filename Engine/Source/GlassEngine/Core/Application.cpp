#include "GlassEngine/Utilities/MemoryProfiler.h"
#include "gepch.h"
#include "Application.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Texture.h"
#include "GlassEngine/Utilities/Timer.h"
#include <stdexcept>
#include <iostream>

namespace ge {
	Application::Application(const ApplicationSpecification& specs) : _specs(specs) {
		_threadManager = mem::CreateScope<ThreadManager>(1);
	}

	Application::~Application() { _layerStack.Clear(); }
	bool Application::Running() const { return !_forceClose; }

	void Application::Update(float deltaTime) {
		{
			for (auto& layer : _layerStack)
				layer->OnUpdate(deltaTime);
		}
	}

	void Application::DrawImGui()
	{
		for (auto& layer : _layerStack) {
			layer->OnImGuiRender();
		}
	}
}