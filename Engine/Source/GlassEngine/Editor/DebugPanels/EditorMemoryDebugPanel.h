#pragma once
#include "GlassEngine/Editor/EditorPanel.h"

namespace ge::editor {
	class EditorMemoryDebugPanel : public EditorPanel {
	public:
		virtual void Draw() override;
	};
}