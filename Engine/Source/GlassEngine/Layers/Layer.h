#pragma once

#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Console.h"

namespace ge {
	class Layer {
	public:
		Layer(const char* debugName = "Layer") : _debugName(debugName) {
			GE_ADD_CONSOLE_COMMAND(_debugName, "showPanel", [this](const GEVector<GEString>& args) { GetPanelManager().ShowPanel(args[0]); }, "showPanel <panelName>");
			GE_ADD_CONSOLE_COMMAND(_debugName, "hidePanel", [this](const GEVector<GEString>& args) { GetPanelManager().DisablePanel(args[0]); }, "hidePanel <panelName>");
			GE_ADD_CONSOLE_COMMAND(_debugName, "printAllRegisteredPanels", [this](const GEVector<GEString>& args) { GetPanelManager().PrintAllPanels(); });
		}

		virtual ~Layer() = default;
		virtual void OnAttach() { RegisterLayerPanels(); }
		virtual void OnDetach() {}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnImGuiRender() { for (auto& [name, panel] : _panelManager) { panel->Draw(); } }
		virtual void RegisterConsoleCommands() {}
		virtual void RegisterLayerPanels() {}
		inline const char* GetDebugName() const { return _debugName.c_str(); }
		inline editor::PanelManager& GetPanelManager() { return _panelManager; }
	private:
		GEString _debugName = "Layer";
		editor::PanelManager _panelManager;
	};
}