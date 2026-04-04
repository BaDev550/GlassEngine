#pragma once
#include <iostream>
#include "GlassEngine/Core/Memory.h"

namespace ge::renderer {
	enum class GraphicsAPI {
		Vulkan,
		OpenGL,
		DirectX11
	};

	class RenderAPI {
	public:

		static inline GraphicsAPI GetAPI() { return _graphicsAPI; }
	private:
		static inline GraphicsAPI _graphicsAPI = GraphicsAPI::Vulkan;
	};
}