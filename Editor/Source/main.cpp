#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Core/Memory.h>
#include <GlassEngine/Core/Core.h>
#include <GlassEngine/Renderer/Renderer.h>
#include <GlassEngine/GUI/GUIConsole.h>

#include <imgui.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("EditorLayer") {}
	virtual void OnAttach() override {
		ge::Console::Get().AddCommand("editor", "ping", [](const GEVector<std::string>& args) { GE_APPLICATION_INFO("Pong!"); });
	}

	virtual void OnDetach() override {}
	virtual void OnUpdate(float deltaTime) override {
		ge::renderer::Renderer3D::BeginDefaultPass(); // TODO (badev): Move this into scene renderer

		ge::renderer::Renderer3D::EndDefaultPass();
	}

	virtual void OnImGuiRender() override {
		_console.OnImGuiRender();
	}
private:
	ge::gui::Console _console;
};

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& createInfo) : ge::Application(createInfo) {
		GE_APPLICATION_INFO("EditorApp created!");
		PushLayer(new EditorLayer());
		GetEditorAssetManager().CompileIntoPakFile("Assets.pak");
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationSpecification& createInfo) {
		return new EditorApp(createInfo);
	}
}