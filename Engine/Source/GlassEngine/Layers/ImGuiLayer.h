#pragma once
#include "GlassEngine/Layers/Layer.h"

namespace ge {
	class ImGuiLayer : public Layer {
	public:
		ImGuiLayer() : Layer("LAYER_Imgui") {}
		virtual ~ImGuiLayer() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		static ImGuiLayer* Create();
	};
}