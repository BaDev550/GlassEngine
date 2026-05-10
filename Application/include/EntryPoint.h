#pragma once

#include "Application.h"
extern ge::Application* CreateApplication(const ge::ApplicationCommandLineArgs& args);

int main(int argc, char** argv) {
	{
		ge::Application* app = ge::CreateApplication({ argc, argv });
		delete app;
	}
	return 0;
}