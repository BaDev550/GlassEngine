#pragma once
#include "GlassEngine/Memory/Memory.h"

namespace ge::editor {
	class EditorPanel {
	public:
		virtual ~EditorPanel() = default;
		virtual void Draw() = 0;
	};
}