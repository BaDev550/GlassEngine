#pragma once
#include <Core.h>
#include <Logger.h>
#include <Application.h>

#include "IEngineSystem.h"
#include "Graphics/Interfaces/IRenderSystem.h"

namespace ge {
#define GE_CORE_TRACE(msg, ...) ::ge::Engine::Get().GetLogger().trace(msg, ##__VA_ARGS__)
#define GE_CORE_INFO(msg, ...) ::ge::Engine::Get().GetLogger().info(msg, ##__VA_ARGS__)
#define GE_CORE_WARN(msg, ...) ::ge::Engine::Get().GetLogger().warn(msg, ##__VA_ARGS__)
#define GE_CORE_ERROR(msg, ...) ::ge::Engine::Get().GetLogger().error(msg, ##__VA_ARGS__)
#define GE_CORE_CRITICAL(msg, ...) ::ge::Engine::Get().GetLogger().critical(msg, ##__VA_ARGS__)

#define GE_GRAPHICS_TRACE(msg, ...) ::ge::Engine::Get().GetRenderSystem()->GetLogger().trace(msg, ##__VA_ARGS__)
#define GE_GRAPHICS_INFO(msg, ...) ::ge::Engine::Get().GetRenderSystem()->GetLogger().info(msg, ##__VA_ARGS__)
#define GE_GRAPHICS_WARN(msg, ...) ::ge::Engine::Get().GetRenderSystem()->GetLogger().warn(msg, ##__VA_ARGS__)
#define GE_GRAPHICS_ERROR(msg, ...) ::ge::Engine::Get().GetRenderSystem()->GetLogger().error(msg, ##__VA_ARGS__)
#define GE_GRAPHICS_CRITICAL(msg, ...) ::ge::Engine::Get().GetRenderSystem()->GetLogger().critical(msg, ##__VA_ARGS__)
	enum class EngineMode : uint8_t {
		Editor = 0,
		Runtime
	};

	struct EngineSpec {
		Application* application = nullptr;
		EngineMode mode = EngineMode::Editor;
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

		graphics::IRenderSystem* GetRenderSystem() { return _renderSystem; }
	private:
		static Engine* _instance;
		EngineSpec _specs;

		graphics::IRenderSystem* _renderSystem;

		mem::Ref<Logger> _logger_engine;
	};
}