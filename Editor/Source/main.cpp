#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Core/Memory.h>
#include <GlassEngine/Core/Core.h>
#include <GlassEngine/Renderer/Texture.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("EditorLayer") {}
	virtual void OnAttach() override {}
	virtual void OnDetach() override {}
	virtual void OnUpdate(float deltaTime) override {}
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