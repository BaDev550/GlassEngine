#pragma once

#include "Application.h"
extern ge::Application* ge::CreateApplication(const ge::ApplicationCreateInfo& createInfo);

int main() {
	auto app = ge::CreateApplication({ "GlassEngineApp", 1280, 720 });
	app->Run();
	delete app;
}