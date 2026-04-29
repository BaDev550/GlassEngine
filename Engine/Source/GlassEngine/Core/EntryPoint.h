#pragma once

#include "Application.h"
#include "Engine.h"
extern ge::Application* ge::CreateApplication(const ge::ApplicationCommandLineArgs& createInfo);

int main(int argc, char** argv) {
	ge::Logger::Init();
	{
		ge::Application* app = ge::CreateApplication({argc, argv});

		ge::EngineSpecification GlassEngineSpecs{};
		GlassEngineSpecs.application = app;
		GlassEngineSpecs.IDE = ge::EngineIDE::VS22;
		GlassEngineSpecs.mode = ge::EngineMode::Editor;

		ge::Engine GlassEngine(GlassEngineSpecs);
		GlassEngine.Run();
		GlassEngine.Destroy();
	}
	ge::Logger::Destroy();
	return 0;
}