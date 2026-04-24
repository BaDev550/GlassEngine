#pragma once
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Memory/Memory.h"

#include "EditorPanel.h"
#include <functional>

namespace ge::editor {
	class PanelManager {
	public:
		PanelManager() = default;
		~PanelManager();
		template<typename T, typename... Args>
		void RegisterPanel(const GEString& panelName, Args&&... args);
		void RemovePanel(const GEString& panelName);
		EditorPanel* ShowPanel(const GEString& panelName);
		void DisablePanel(const GEString& panelName);
		void PrintAllPanels();
		GEUnorderedMap<GEString, EditorPanel*>::iterator begin() { return _activePanels.begin(); }
		GEUnorderedMap<GEString, EditorPanel*>::iterator end() { return _activePanels.end(); }
		GEUnorderedMap<GEString, EditorPanel*>::const_iterator begin() const { return _activePanels.begin(); }
		GEUnorderedMap<GEString, EditorPanel*>::const_iterator end() const { return _activePanels.end(); }
	private:
		GEUnorderedMap<GEString, std::function<EditorPanel*()>> _registeredPanels;
		GEUnorderedMap<GEString, EditorPanel*> _activePanels;
	};

	template<typename T, typename... Args>
	void PanelManager::RegisterPanel(const GEString& panelName, Args&&... args)
	{
		static_assert(std::is_base_of<EditorPanel, T>::value, "T Needs to be child class of EditorPanel");
		_registeredPanels[panelName] = [args...]()->EditorPanel* { return new T(args...); };
	}
}