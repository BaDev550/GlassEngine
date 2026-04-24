#include "gepch.h"
#include "PanelStack.h"

namespace ge::editor {
	PanelManager::~PanelManager()
	{
		for (auto& [name, panel] : _activePanels) {
			delete panel;
			panel = nullptr;
		}
		_activePanels.clear();
	}

	void PanelManager::RemovePanel(const GEString& panelName)
	{
		_activePanels.erase(panelName);
	}

	EditorPanel* PanelManager::ShowPanel(const GEString& panelName)
	{
		if (_activePanels.find(panelName) != _activePanels.end()) {
			return nullptr;
		}

		auto it = _registeredPanels.find(panelName);
		if (it != _registeredPanels.end()) {
			_activePanels[panelName] = it->second();
			GE_CORE_TRACE("Panel '{}' created and shown.", panelName);
			return _activePanels[panelName];
		}
		else {
			GE_CORE_WARN("Attempted to show panel '{}', but it is not registered!", panelName);
			return nullptr;
		}
	}

	void PanelManager::DisablePanel(const GEString& panelName)
	{
		auto it = _activePanels.find(panelName);
		if (it != _activePanels.end()) {
			delete it->second;
			_activePanels.erase(it);
			GE_CORE_TRACE("Panel '{0}' disabled and destroyed.", panelName);
		}
	}
	void PanelManager::PrintAllPanels()
	{
		for (const auto& [name, panel] : _registeredPanels) {
			GE_CORE_INFO("Panel: {}", name);
		}
	}
}