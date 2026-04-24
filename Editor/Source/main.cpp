#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Memory/Ref.h>
#include <GlassEngine/Core/Core.h>
#include <GlassEngine/Renderer/Renderer.h>
#include <GlassEngine/Scene/Scene.h>
#include <GlassEngine/Editor/EditorConsole.h>
#include <GlassEngine/Editor/EditorECSDebugPanel.h>

#include <imgui.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("EditorLayer") {}
	virtual void OnAttach() override {
		_scene = ge::mem::Ref<ge::Scene>::Create("Test Scene");

		GetPanelManager().RegisterPanel<ge::editor::Console>("E_c");
		GetPanelManager().RegisterPanel<ge::editor::EditorECSDebugPanel>("E_ecs", _scene.Get());

		GE_EXECUTE_CONSOLE_COMMAND("EditorLayer.showPanel E_c");
		GE_EXECUTE_CONSOLE_COMMAND("EditorLayer.showPanel E_ecs");
	}

	virtual void OnDetach() override {}
	virtual void OnUpdate(float deltaTime) override {
		ge::renderer::Renderer3D::BeginDefaultPass(); // TODO (badev): Move this into scene renderer

		_scene->OnEditorUpdate(0);

		ge::renderer::Renderer3D::EndDefaultPass();
	}

	virtual void OnImGuiRender() override {}
private:
	ge::mem::Ref<ge::Scene> _scene;
};

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& createInfo) : ge::Application(createInfo) {
		GE_APPLICATION_INFO("EditorApp created!");
		PushLayer(new EditorLayer());
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationSpecification& createInfo) {
		return new EditorApp(createInfo);
	}
}