#include <Core.h>
#include <EntryPoint.h>

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& specs) : ge::Application(specs) {
		std::cout << "DLL Application running";
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