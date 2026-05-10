#pragma once
#include <Core.h>
#include <Logger.h>
#include <Application.h>

namespace ge {
	struct EngineSpec {
		Application* application = nullptr;

		GEString name = "Glass Engine v1";
	};

	class GE_API Engine {
	public:
		Engine(const EngineSpec& specs);
		~Engine();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;
		static Engine& Get() { return *_instance; }

		void Run();
		void Destroy();

		Application& GetApplication() { return *_specs.application; }
		Window& GetApplicationWindow() { return GetApplication().GetWindow(); }
		Logger& GetLogger() { return *_logger_engine; }
		EngineSpec GetEngineSpecs() const { return _specs; }
		ApplicationSpecification GetApplicationSpecs() { return GetApplication().GetSpecs(); }
	private:
		static Engine* _instance;
		EngineSpec _specs;

		mem::Ref<Logger> _logger_engine;
	};
}