#include <imgui.h>
#include <string>
#include <string.h>
#include <filesystem>
#include "GlassEngine/Asset/Asset.h"
#include "GlassEngine/Editor/EditorPanel.h"
#include <nfd.hpp>

namespace ge::editor {
    class ModelImportPanel : public EditorPanel {
    public:
        ModelImportPanel() {
            memset(sourcePathBuffer, 0, sizeof(sourcePathBuffer));
            memset(targetPathBuffer, 0, sizeof(targetPathBuffer));
            importData = ImportAssetData();
            importData.sourceMeshSpecs = &meshSpec;
        }

        virtual void Draw() override {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::Begin("Import Model Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

                ImGui::Text("File Paths");

                ImGui::InputText("Source Path", sourcePathBuffer, sizeof(sourcePathBuffer));
                ImGui::SameLine();
                if (ImGui::Button("Browse##Source")) {
                    nfdchar_t* outPath = nullptr;
                    nfdfilteritem_t filterItem[1] = { { "GLTF Models", "gltf,glb" } };

                    if (NFD::OpenDialog(outPath, filterItem, 1) == NFD_OKAY) {
                        snprintf(sourcePathBuffer, sizeof(sourcePathBuffer), "%s", outPath);
                        NFD::FreePath(outPath);
                    }
                }

                ImGui::InputText("Target Path", targetPathBuffer, sizeof(targetPathBuffer));
                ImGui::SameLine();
                if (ImGui::Button("Browse##Target")) {
                    nfdchar_t* outPath = nullptr;

                    if (NFD::PickFolder(outPath, nullptr) == NFD_OKAY) {
                        snprintf(targetPathBuffer, sizeof(targetPathBuffer), "%s", outPath);
                        NFD::FreePath(outPath);
                    }
                }

                ImGui::Separator();

                std::filesystem::path srcPath(sourcePathBuffer);
                std::filesystem::path tgtPath = targetPathBuffer / srcPath.filename();
                tgtPath.replace_extension(GE_ASSET_EXTENSION);

                if (!srcPath.empty()) {
                    ImGui::Text("Import Settings");
                    ImGui::Checkbox("Flip UVs", &meshSpec.flipUVs);
                    ImGui::Checkbox("Import Materials", &meshSpec.loadMaterials);

                    ImGui::Separator();
                    if (ImGui::Button("Import", ImVec2(120, 0))) {
                        auto importedMesh = ge::AssetManager::GetOrImportAsset<ge::renderer::StaticMesh>(srcPath, tgtPath, importData);
                        if (importedMesh) {
                            GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.hidePanel modelImporter");
                        }
                        else {
                        }
                    }
                    ImGui::SameLine();
                }

                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.hidePanel modelImporter");
                }

                ImGui::End();
            }
        }
    private:
        bool bIsOpen = false;
        char sourcePathBuffer[512] = "";
        char targetPathBuffer[512] = "";
        ImportAssetData importData;
        renderer::SourceMeshSpec meshSpec;
    };
}