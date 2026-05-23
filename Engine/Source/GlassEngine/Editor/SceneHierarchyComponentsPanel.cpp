#include "gepch.h"
#include "SceneHierarchyPanel.h"
#include "GlassEngine/Scene/Scene.h"
#include "GlassEngine/Asset/AssetManager.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace ge::editor {
	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity* entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (entity->HasComponent<T>())
		{
			auto& component = entity->GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar(
			);
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity->RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DisplayComponentSettings(Entity* entity)
	{
		ImGui::PopItemWidth();
		DrawComponent<TransformComponent>("Transform", entity, [](TransformComponent& component)
			{
				ImGui::DragFloat3("Translation", glm::value_ptr(component.position), 0.1f);
				ImGui::DragFloat3("Rotation", glm::value_ptr(component.rotation), 0.1f);
				ImGui::DragFloat3("Scale", glm::value_ptr(component.scale), 0.1f);
			});

        DrawComponent<PointLightComponent>("Point Light", entity, [](PointLightComponent& component)
            {

            });

        DrawComponent<RigidBodyComponent>("Rigid Body", entity, [](RigidBodyComponent& component)
            {
                const char* bodyTypes[] = { "Static", "Dynamic" };
                int currentType = (int)component.bodyType;
                if (ImGui::Combo("Body Type", &currentType, bodyTypes, 2))
                    component.bodyType = (RigidBodyType)currentType;

                if (component.bodyType == RigidBodyType::Dynamic)
                {
                    ImGui::DragFloat("Mass", &component.mass, 0.1f, 0.001f, 10000.0f);
                    ImGui::Checkbox("Use Gravity", &component.useGravity);
                    ImGui::DragFloat("Linear Damping", &component.linearDamping, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Angular Damping", &component.angularDamping, 0.01f, 0.0f, 1.0f);
                    ImGui::Separator();
                    ImGui::Text("Lock Rotation");
                    ImGui::Checkbox("X", &component.lockAngularX);
                    ImGui::SameLine();
                    ImGui::Checkbox("Y", &component.lockAngularY);
                    ImGui::SameLine();
                    ImGui::Checkbox("Z", &component.lockAngularZ);
                }

                if (component.actor)
                {
                    ImGui::Separator();
                    ImGui::TextDisabled("Actor: active");
                }
                else
                {
                    ImGui::Separator();
                    ImGui::TextDisabled("Actor: not spawned");
                }
            });

        DrawComponent<BoxColliderComponent>("Box Collider", entity, [](BoxColliderComponent& component)
            {
                ImGui::DragFloat3("Half Extents", glm::value_ptr(component.halfExtents), 0.05f, 0.001f, 1000.0f);
                ImGui::DragFloat3("Offset", glm::value_ptr(component.offset), 0.05f);
            });

        DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](SphereColliderComponent& component)
            {
                ImGui::DragFloat("Radius", &component.radius, 0.05f, 0.001f, 1000.0f);
                ImGui::DragFloat3("Offset", glm::value_ptr(component.offset), 0.05f);
            });

        DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, [](CapsuleColliderComponent& component)
            {
                ImGui::DragFloat("Radius", &component.radius, 0.05f, 0.001f, 1000.0f);
                ImGui::DragFloat("Half Height", &component.halfHeight, 0.05f, 0.001f, 1000.0f);
                ImGui::DragFloat3("Offset", glm::value_ptr(component.offset), 0.05f);
            });

		DrawComponent<StaticMeshComponent>("Mesh", entity, [](StaticMeshComponent& component)
			{
                auto Handles = AssetManager::Editor_GetLoadedAssetsWithType(AssetType::StaticMesh);
                std::vector<AssetHandle> handles;
                std::vector<std::string> names;

                std::string currentName = "None";

                for (const auto& [id, asset] : Handles) {
                    handles.push_back(id);
                    std::string name = AssetManager::GetMetadata(id).path.stem().string();
                    names.push_back(name);

                    if (component.meshHandle == id) {
                        currentName = name;
                    }
                }

                if (ImGui::BeginCombo("Select Mesh", currentName.c_str())) {
                    bool isNoneSelected = !component.meshHandle;
                    if (ImGui::Selectable("None", isNoneSelected)) {
                        component.meshHandle = 0;
                        component.materialTable = nullptr;
                    }
                    if (isNoneSelected) {
                        ImGui::SetItemDefaultFocus();
                    }

                    for (size_t i = 0; i < handles.size(); i++) {
                        bool isSelected = (component.meshHandle == handles[i]);

                        if (ImGui::Selectable(names[i].c_str(), isSelected)) {
                            if (component.meshHandle != handles[i]) {
                                component.meshHandle = handles[i];
                                component.materialTable = nullptr; 
                            }
                        }

                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if (!component.meshHandle) {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Mesh not loaded.");
                }
                else {
                    int lodIndex = component.lodLevel;
                    ImGui::Checkbox("Visible", &component.isVisible);
                    ImGui::Checkbox("Calculate LOD", &component.calculateLOD);
                    if (ImGui::DragInt("Current LOD", &lodIndex, 1.0f, 0, 3)) {
                        component.lodLevel = lodIndex;
                    }

                    auto staticMesh = AssetManager::GetAsset<renderer::StaticMesh>(component.meshHandle);
                    if (staticMesh && ImGui::TreeNode("Materials")) {

                        auto defaultMaterialTable = staticMesh->GetMaterialTable();
                        auto& activeMaterials = component.materialTable ? component.materialTable->GetMaterials() : defaultMaterialTable->GetMaterials();

                        if (component.materialTable) {
                            if (ImGui::Button("Clear Overrides")) {
                                component.materialTable = nullptr;
                            }
                        }

                        auto matAssets = AssetManager::Editor_GetLoadedAssetsWithType(AssetType::Material);
                        std::vector<AssetHandle> matHandles;
                        std::vector<std::string> matNames;
                        for (const auto& [id, asset] : matAssets) {
                            matHandles.push_back(id);
                            matNames.push_back(AssetManager::GetMetadata(id).path.stem().string());
                        }

                        for (size_t i = 0; i < activeMaterials.size(); i++) {
                            auto currentMat = activeMaterials[i];

                            std::string currentMatName = "None";
                            if (currentMat) {
                                AssetHandle handle = currentMat->_assetHandle;
                                auto it = std::find(matHandles.begin(), matHandles.end(), handle);
                                if (it != matHandles.end()) {
                                    currentMatName = matNames[std::distance(matHandles.begin(), it)];
                                }
                                else {
                                    currentMatName = "Material " + std::to_string(i);
                                }
                                if (ImGui::CollapsingHeader(currentMatName.c_str())) {
                                    float roughness = currentMat->GetMaterialData().roughness;
                                    if (ImGui::DragFloat("Roughness", &roughness, 0.1f))
                                        currentMat->SetRoughness(roughness);
                                    float metallic = currentMat->GetMaterialData().metallic;
                                    if (ImGui::DragFloat("Metallic", &metallic, 0.1f))
                                        currentMat->SetMetallic(metallic);
                                }
                            }

                            ImGui::PushID(static_cast<int>(i));
                            std::string slotLabel = std::format("Slot {}", i);

                            if (ImGui::BeginCombo(slotLabel.c_str(), currentMatName.c_str())) {
                                for (size_t j = 0; j < matHandles.size(); j++) {
                                    bool isSelected = (currentMatName == matNames[j]);
                                    if (ImGui::Selectable(matNames[j].c_str(), isSelected)) {

                                        if (!component.materialTable) {
                                            component.materialTable = mem::Ref<renderer::MaterialTable>::Create();
                                            for (size_t m = 0; m < defaultMaterialTable->GetMaterials().size(); m++) {
                                                component.materialTable->AddMaterial(static_cast<uint32_t>(m), defaultMaterialTable->GetMaterials()[m]);
                                            }
                                        }

                                        auto newMat = AssetManager::GetAsset<renderer::MaterialAsset>(matHandles[j]);
                                        component.materialTable->AddMaterial(static_cast<uint32_t>(i), newMat);
                                    }

                                    if (isSelected) ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                }
			});
	}
}