#pragma once
#include "GlassEngine/Core/Core.h"
#include "Application.h"

#include "GlassEngine/Core/Input.h"

#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Utilities/Logger.h"
#include "GlassEngine/Utilities/Console.h"
#include "GlassEngine/Utilities/Profiler.h"
#include "GlassEngine/Asset/AssetManager.h"

namespace ge {
	enum class EngineIDE : uint8_t {
		VS22,
		VSC,
		VIM,
		CLion
	};

	enum class EngineMode : uint8_t {
		Editor = 0,
		Runtime
	};

	struct EngineSpecification {
		GEString name = "Glass Engine";
		EngineIDE IDE = EngineIDE::VS22;
		EngineMode mode = EngineMode::Editor;
		Application* application = nullptr;
	};

	class Engine {
	public:
		Engine(const EngineSpecification& specs);
		~Engine();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;
		static Engine& Get() { return *_instance; }

		void Run();
		void Destroy();

		Application* GetApplication() const { return _specs.application; }
		Window& GetApplicationWindow() { return *_window; }
		ThreadManager& GetApplicationThreadManager() { return GetApplication()->GetThreadManager(); }
		ApplicationSpecification GetApplicationSpecs() { return GetApplication()->GetSpecs(); }
		const float GetDeltaTime() const { return _deltaTime; }

		InputManager& GetInputManager() { return *_inputManager; }
		EngineSpecification GetSpecs() { return _specs; }
	private:
		EngineSpecification _specs;
		static Engine* _instance;

		ImGuiLayer* _imGuiLayer;

		float _lastTime = 0.0f;
		float _deltaTime = 0.0f;
		// TODO(badev): Get all the input / render objects to here

		mem::Scope<Window> _window;
		mem::Scope<InputManager> _inputManager;
	};
}