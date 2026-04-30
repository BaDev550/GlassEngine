#pragma once

#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Scene/Entity.h"
#include "GlassEngine/Scene/Scene.h"
#include "GlassEngine/Editor/EditorPanel.h"

// Forked from https://github.com/BaDev550/Tourqe-Advanced-Game-Engine
namespace ge::editor {
	class SceneHierarchyPanel : public EditorPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const mem::Ref<Scene>& scene);

		void SetContext(const mem::Ref<Scene>& scene);

		virtual void Draw() override;

		Entity* GetSelectedEntity() const { return _selectionContext; }
		void SetSelectedEntity(Entity* entity);
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);
		void DisplayComponentSettings(Entity* entity);

		void DrawEntityNode(Entity* entity);
		void DrawComponents(Entity* entity);
	private:
		mem::Ref<Scene> _context;
		Entity* _selectionContext;
	};
}