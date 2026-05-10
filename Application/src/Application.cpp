#include "Application.h"

namespace ge {
	Application::Application(const ApplicationSpecification& specs) : _specs(specs) {
		_window = mem::CreateScope<Window>(WindowSpec(_specs.title, _specs.width, _specs.height, false));
		_globalSink = mem::Ref<Sink>::Create("GLOBAL_APPLICATION_SINK", GEVector<logLevel>({ LL_Info, LL_Error, LL_Warn }));
	}

	void Application::Update(float deltaTime)
	{
		_window->PollEvents();
	}

	void Application::Close() {
		_forceClose = true;
	}
}