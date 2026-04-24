#pragma once

namespace ge {
	class Scene;
}
namespace ge::gui {
	class GUIECSDebugPanel { // TODO (badev): add a panel manager so we can create panels in application and not have to worry about their lifetime
	public:
		GUIECSDebugPanel(Scene* scene);
		bool IsOpen() const { return _drawPanel; }
		void Toggle() { _drawPanel = !_drawPanel; }
		void OnImGuiRender();
	private:
		bool _drawPanel = false;
		Scene* _scene;
	};
}