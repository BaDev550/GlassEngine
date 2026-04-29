#pragma once

#include "Application.h"
extern ge::Application* ge::CreateApplication(const ge::ApplicationCommandLineArgs& createInfo);

int main(int argc, char** argv) {
	{
		auto app = ge::CreateApplication({argc, argv});
		app->Run();
		delete app;
	}
	return 0;
}