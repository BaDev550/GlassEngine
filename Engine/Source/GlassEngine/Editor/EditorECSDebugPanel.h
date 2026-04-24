#pragma once
#include "EditorPanel.h"

namespace ge {
	class Scene;
	class Entity;
}
namespace ge::editor {
	class EditorECSDebugPanel : public EditorPanel {
	public:
		EditorECSDebugPanel(Scene* scene);
		virtual void Draw() override;
	private:
		Scene* _scene;
		Entity* _selectedEntity = nullptr;
	};
}