#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Memory/Ref.h>
#include <GlassEngine/Core/Core.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("EditorLayer") {
		GE_APPLICATOIN_INFO("EditorLayer created!");
	}
	virtual void OnAttach() override {
		GE_APPLICATOIN_INFO("EditorLayer attached!");
	}
	virtual void OnDetach() override {
		GE_APPLICATOIN_INFO("EditorLayer detached!");
	}
	virtual void OnUpdate(float deltaTime) override {}
};

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& createInfo) : ge::Application(createInfo) {
		GE_APPLICATOIN_INFO("EditorApp created!");
		PushLayer(new EditorLayer());
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationSpecification& createInfo) {
		return new EditorApp(createInfo);
	}
}