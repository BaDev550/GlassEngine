#include "gepch.h"
#include "SceneHierarchyPanel.h"
#include "GlassEngine/Scene/Components/BaseComponents.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

#include <cstring>

namespace ge::editor {
	SceneHierarchyPanel::SceneHierarchyPanel(const mem::Ref<Scene>& context) {
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const mem::Ref<Scene>& context) {
		_context = context;
		_selectionContext = nullptr;
	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity* entity) {
		_selectionContext = entity;
	}

	void SceneHierarchyPanel::Draw()
	{
		ImGui::Begin("Scene Hierarchy");

		if (_context)
		{
			_context->GetRegistry().view<IdentityComponent, TransformComponent>().each(
				[&](entt::entity entityID, IdentityComponent& id, TransformComponent& transform)
				{
					Entity* entity = _context->GetEntityByID(id.id);
					DrawEntityNode(entity);
				});


			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				_selectionContext = nullptr;

			if (ImGui::BeginPopupContextWindow(0, 1))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					_context->CreateEntity("Empty Entity");

				ImGui::EndPopup();
			}

		}
		ImGui::End();

		ImGui::Begin("Properties");
		if (_selectionContext) { DrawComponents(_selectionContext); }
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity* entity)
	{
		bool entityDeleted = false;
		if (entityDeleted) {
			_context->DestroyEntity(entity);
			if (_selectionContext == entity)
				_selectionContext = nullptr;
		}

		auto& tag = entity->GetComponent<IdentityComponent>().name;
		ImGuiTreeNodeFlags flags = ((_selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			_selectionContext = entity;
		}

		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity* entity)
	{
		if (entity->HasComponent<IdentityComponent>())
		{
			auto& tag = entity->GetComponent<IdentityComponent>().name;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			snprintf(buffer, sizeof(buffer), "%s", tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}

			if (ImGui::Button("Delete")) {
				_selectionContext = nullptr;
				_context->DestroyEntity(entity);
			}
		}
		if (!_selectionContext || !entity || !entity->GetRegisteredScene()) return;

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<StaticMeshComponent>("Mesh");
			DisplayAddComponentEntry<PointLightComponent>("Point Light");
			DisplayAddComponentEntry<SpotLightComponent>("Spot Light");
			DisplayAddComponentEntry<DirectionalLightComponent>("Directional Light");

			ImGui::EndPopup();
		}

		ImGui::Text("Uniqe ID: %s", std::to_string(entity->GetComponent<IdentityComponent>().id).c_str());

		DisplayComponentSettings(entity);
	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName) {
		if (!_selectionContext->HasComponent<T>()) {
			if (ImGui::MenuItem(entryName.c_str()))
			{
				_selectionContext->AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}