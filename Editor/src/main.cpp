#include <Core.h>
#include <EntryPoint.h>

#include <Engine.h>

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& specs) : ge::Application(specs) {}

	virtual void OnStart() override {
		ge::Engine::Get().GetLogger().info("Test");
		ge::Engine::Get().GetLogger().warn("Test");
		ge::Engine::Get().GetLogger().error("Test");
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationCommandLineArgs& args) {
		ApplicationSpecification specs{};
		specs.title = "Glass Editor";
		specs.width = 1280;
		specs.height = 720;
		specs.commandLineArgs = args;
		return new EditorApp(specs);
	}
}