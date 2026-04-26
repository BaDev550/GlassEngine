#include "gepch.h"
#include "LayerStack.h"

namespace ge {
	LayerStack::~LayerStack() {
		Clear();
	}

	void LayerStack::PushLayer(Layer* layer) {
		_layers.emplace(_layers.begin() + _layerInsertIndex, layer);
		layer->OnAttach();
		_layerInsertIndex++;
	}
	void LayerStack::PushOverlay(Layer* overlay) {
		_layers.emplace_back(overlay);
		overlay->OnAttach();
	}
	void LayerStack::PopLayer(Layer* layer) {
		auto it = std::find(_layers.begin(), _layers.begin() + _layerInsertIndex, layer);
		if (it != _layers.begin() + _layerInsertIndex) {
			layer->OnDetach();
			_layers.erase(it);
			_layerInsertIndex--;
		}
	}
	void LayerStack::PopOverlay(Layer* overlay) {
		auto it = std::find(_layers.begin() + _layerInsertIndex, _layers.end(), overlay);
		if (it != _layers.end()) {
			overlay->OnDetach();
			_layers.erase(it);
		}
	}

	void LayerStack::Clear() {
		if (_layers.empty()) return;
		for (Layer* layer : _layers) {
			if (layer) {
				_layerInsertIndex--;
				layer->OnDetach();
				delete layer;
				layer = nullptr;
			}
		}
		_layers.clear();
	}
}