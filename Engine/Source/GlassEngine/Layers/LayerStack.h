#pragma once
#include <vector>
#include "GlassEngine/Editor/PanelStack.h"
#include "Layer.h"

namespace ge {
	class LayerStack {
	public:
		LayerStack() = default;
		~LayerStack();
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);
		std::vector<Layer*>::iterator begin() { return _layers.begin(); }
		std::vector<Layer*>::iterator end() { return _layers.end(); }
		std::vector<Layer*>::const_iterator begin() const { return _layers.begin(); }
		std::vector<Layer*>::const_iterator end() const { return _layers.end(); }
	private:
		std::vector<Layer*> _layers;
		unsigned int _layerInsertIndex = 0;
	};
}