#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationCreateInfo& createInfo) : ge::Application(createInfo) {
		std::cout << "EditorApp created!" << std::endl;
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationCreateInfo& createInfo) {
		return new EditorApp(createInfo);
	}
}