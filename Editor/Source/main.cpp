#include <iostream>
#include <imgui.h>
#include <GlassEngine/Core/EntryPoint.h>

#include "EditorLayer.h"

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& createInfo) : ge::Application(createInfo) {
		GE_APPLICATION_INFO("EditorApp created!");
	}

	virtual void Init() override {
		PushLayer(new ge::editor::EditorLayer());
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationCommandLineArgs& args) {
		ApplicationSpecification appSpecs{};
		appSpecs.title = GEString("Glass Editor");
		appSpecs.width = 1280;
		appSpecs.height = 720;
		appSpecs.commandLineArgs = args;
		return new EditorApp(appSpecs);
	}
}