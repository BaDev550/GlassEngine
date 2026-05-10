#pragma once

#include <Application.h>
#include "Engine.h"
extern ge::Application* CreateApplication(const ge::ApplicationCommandLineArgs& args);

int main(int argc, char** argv) {
	{
		ge::Application* app = ge::CreateApplication({ argc, argv });
		ge::Engine* ge = new ge::Engine(ge::EngineSpec(app));
		ge->Run();
		ge->Destroy();
		delete ge;
	}
	return 0;
}