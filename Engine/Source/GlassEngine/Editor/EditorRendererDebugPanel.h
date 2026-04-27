#pragma once
#include "EditorPanel.h"

namespace ge::editor {
	class EditorRendererDebugPanel : public EditorPanel {
	public:
		virtual void Draw() override;
	};
}