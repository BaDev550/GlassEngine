#include "Engine.h"

#include "Renderer/Interfaces/IRenderSystem.h"

namespace ge {
	Engine* Engine::_instance = nullptr;
	Engine::Engine(const EngineSpec& specs) : _specs(specs) {
		GE_ASSERT(!_instance, "Engine already exists!");
		_instance = this;
		_logger_engine = mem::Ref<Logger>::Create("ENGINE", GetApplication().GetGlobalSink());

		_specs.application->OnStart();

		AddSystem<renderer::IRenderSystem>("");
	}

	Engine::~Engine()
	{
	}

	void Engine::Run()
	{
		while (!GetApplicationWindow().ShoudClose() && GetApplication().Running()) {

			GetApplication().Update(0.0f);
		}
	}

	void Engine::Destroy()
	{
		delete _specs.application;
		_specs.application = nullptr;
	}
}