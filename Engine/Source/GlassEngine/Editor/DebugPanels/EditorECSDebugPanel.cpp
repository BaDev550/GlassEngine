#include "gepch.h"
#include "EditorECSDebugPanel.h"
#include "GlassEngine/Scene/Scene.h"
#include "GlassEngine/Scene/Components/BaseComponents.h"
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

namespace ge::editor {
	EditorECSDebugPanel::EditorECSDebugPanel(Scene* scene) : _scene(scene) {}

	void EditorECSDebugPanel::Draw() {
		ImGui::Begin("ECS Debug Panel");
		ImGui::Text("Scene: %s", _scene->GetName().c_str());
		if (ImGui::Button("Create Entity")) {
			static int entityCount = 1;
			_scene->CreateEntity(("Entity " + std::to_string(entityCount++)));
		}

		ImGui::Separator();
		if (ImGui::CollapsingHeader("Entities")) {
			for (auto& [id, entity] : _scene->GetEntities()) {
				if (entity->HasComponent<IdentityComponent>()) {
					auto& identity = entity->GetComponent<IdentityComponent>();
					if (ImGui::Selectable(std::format("Entity ID: {}, Name: {}", id.ToString().c_str(), identity.name.c_str()).c_str())) {
						_selectedEntity = entity.Get();
					}
				}
			}
		}
		ImGui::Separator();
		if (_selectedEntity) {
			if (_selectedEntity->HasComponent<IdentityComponent>()) {
				auto& ic = _selectedEntity->GetComponent<IdentityComponent>();
				ImGui::Text("Name: %s", ic.name.c_str());
				ImGui::Text("EntityID: %s", ic.id.ToString().c_str());
			}
			if (_selectedEntity->HasComponent<TransformComponent>()) {
				auto& tc = _selectedEntity->GetComponent<TransformComponent>();
				ImGui::DragFloat3("position", glm::value_ptr(tc.position), 0.1f);
				ImGui::DragFloat3("rotation", glm::value_ptr(tc.rotation), 0.1f);
				ImGui::DragFloat3("scale", glm::value_ptr(tc.scale), 0.1f);
			}
			if (_selectedEntity->HasComponent<StaticMeshComponent>()) {
				auto& smc = _selectedEntity->GetComponent<StaticMeshComponent>();
				ImGui::Checkbox("Is Visible", &smc.isVisible);
				ImGui::Checkbox("Calculate LOD", &smc.calculateLOD);
				int lodLevel = smc.lodLevel;
				if (ImGui::DragInt("LOD level: ", &lodLevel, 1.0f, 0, 3))
					smc.lodLevel = lodLevel;
			}
		}
		ImGui::End();
	}
}