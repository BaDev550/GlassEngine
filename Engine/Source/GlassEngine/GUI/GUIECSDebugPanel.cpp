#include "gepch.h"
#include "GUIECSDebugPanel.h"
#include "GlassEngine/Scene/Scene.h"
#include "GlassEngine/Scene/Components/BaseComponents.h"
#include <imgui.h>

namespace ge::gui {
	GUIECSDebugPanel::GUIECSDebugPanel(Scene* scene) : _scene(scene) {}

	void GUIECSDebugPanel::OnImGuiRender() {
		if (!_drawPanel) return;

		ImGui::Begin("ECS Debug Panel");
		ImGui::Text("Scene: %s", _scene->GetName().c_str());
		if (ImGui::Button("Create Entity")) {
			static int entityCount = 1;
			_scene->CreateEntity(("Entity " + std::to_string(entityCount++)));
		}

		ImGui::Separator();
		for (auto& [id, entity] : _scene->GetEntities()) {
			if (entity->HasComponent<IdentityComponent>()) {
				auto& identity = entity->GetComponent<IdentityComponent>();
				ImGui::Text("Entity ID: %s, Name: %s", id.ToString().c_str(), identity.name.c_str());
			}
		}
		ImGui::End();
	}
}