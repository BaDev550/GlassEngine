#pragma once

#include "Application.h"
extern ge::Application* ge::CreateApplication(const ge::ApplicationSpecification& createInfo);

int main() {
	//_CrtSetBreakAlloc(417);
	//_CrtSetBreakAlloc(290);
	//_CrtSetBreakAlloc(359);
	{
		auto app = ge::CreateApplication({ "GlassEngineApp", ge::ApplicationMode::Editor, 1280, 720 }); // TODO (badev): change this to only passing args
		app->Run();
		delete app;
	}
	_CrtDumpMemoryLeaks();
	return 0;
}