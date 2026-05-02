#include "gepch.h"
#include "ContentBrowserPanel.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "GlassEngine/Scene/SceneSerializer.h"
#include "GlassEngine/Project/Project.h"
#include "GlassEngine/Asset/AssetManager.h"
#include "GlassEngine/Renderer/Renderer.h"

#include <fstream>

namespace ge::editor {
	ContentBrowserPanel::ContentBrowserPanel() {
		_icons.push_back(ge::SourceTextureSerializer::ImportTextureFromFile(ImportAssetData(), (GE_ENGINE_DIR / "Resources/Icons/ContentBrowser/Folder.png")).Cast<renderer::Texture2D>());
		_icons.push_back(ge::SourceTextureSerializer::ImportTextureFromFile(ImportAssetData(), (GE_ENGINE_DIR / "Resources/Icons/ContentBrowser/File.png")).Cast<renderer::Texture2D>());
		_icons.push_back(ge::SourceTextureSerializer::ImportTextureFromFile(ImportAssetData(), (GE_ENGINE_DIR / "Resources/Icons/ContentBrowser/StaticMesh.png")).Cast<renderer::Texture2D>());
		_icons.push_back(ge::SourceTextureSerializer::ImportTextureFromFile(ImportAssetData(), (GE_ENGINE_DIR / "Resources/Icons/ContentBrowser/Texture.png")).Cast<renderer::Texture2D>());
		_icons.push_back(ge::SourceTextureSerializer::ImportTextureFromFile(ImportAssetData(), (GE_ENGINE_DIR / "Resources/Icons/ContentBrowser/Material.png")).Cast<renderer::Texture2D>());

		_directoryIconTextureId = renderer::Renderer3D::GetImGuiTexture("CB_IMGT_DIR", _icons[0]->GetImage());
		_defaultFileIconTextureId = renderer::Renderer3D::GetImGuiTexture("CB_IMGT_DEF", _icons[1]->GetImage());
		_staticMeshFileIconTextureId = renderer::Renderer3D::GetImGuiTexture("CB_IMGT_STM", _icons[2]->GetImage());
		_textureFileIconTextureId = renderer::Renderer3D::GetImGuiTexture("CB_IMGT_TEX", _icons[3]->GetImage());
		_materialFileIconTextureId = renderer::Renderer3D::GetImGuiTexture("CB_IMGT_MAT", _icons[4]->GetImage());
	}

	void ContentBrowserPanel::Init() {
		_baseDirectory = Project::GetAssetDirectory();
		_currentDirectory = _baseDirectory;

		RefreshAssetTree();
	}

	void ContentBrowserPanel::Draw()
	{
		ImGui::Begin("Content Browser");

		if (_currentDirectory != std::filesystem::path(_baseDirectory))
		{
			ImGui::SameLine();
			if (ImGui::Button("<-")) {
				_currentDirectory = _currentDirectory.parent_path();
				RefreshAssetTree();
			}
		}
		ImGui::SameLine();

		if (ImGui::Button("New Folder")) {
			ImGui::OpenPopup("Create New Folder");
		}

		float cellSize = _thumbnailSize + _padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (int i = 0; i < _directoryEntries.size(); i++) {
			auto entry = _directoryEntries[i];
			if (!std::filesystem::exists(entry.path))
				continue;
			
			AssetType assetType = AssetManager::GetMetadata(entry.handle).type;
			ImTextureID fileIcon;
			switch (assetType)
			{
			case ge::AssetType::Unknown: fileIcon = _defaultFileIconTextureId; break;
			case ge::AssetType::SourceMesh: fileIcon = _defaultFileIconTextureId; break;
			case ge::AssetType::SourceTexture: fileIcon = _defaultFileIconTextureId; break;
			case ge::AssetType::SourceShader: fileIcon = _defaultFileIconTextureId; break;
			case ge::AssetType::CompiledAsset: fileIcon = _defaultFileIconTextureId; break;
			case ge::AssetType::Texture: fileIcon = _textureFileIconTextureId; break;
			case ge::AssetType::StaticMesh: fileIcon = _staticMeshFileIconTextureId; break;
			case ge::AssetType::Material: fileIcon = _materialFileIconTextureId; break;
			case ge::AssetType::Scene: fileIcon = _defaultFileIconTextureId; break;
			default:
				break;
			}
			ImTextureID& icon = entry.isDirectory ? _directoryIconTextureId : fileIcon;
			std::string item = entry.path.filename().string();

			ImGui::PushID(item.c_str());
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

			if (ImGui::ImageButton("CBIMAGE", icon, { _thumbnailSize, _thumbnailSize })) {

			}

			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Rename")) {
					static char renameBuffer[128];
					strcpy(renameBuffer, entry.path.filename().string().c_str());
					ImGui::OpenPopup("Rename Item");
				}

				if (ImGui::MenuItem("Import")) {
					AssetManager::GetOrImportAsset<Asset>(entry.path);
				}

				if (ImGui::MenuItem("Delete")) {
					std::filesystem::path target = _currentDirectory / entry.path.filename();
					if (std::filesystem::exists(target)) {
						std::filesystem::remove_all(target);
						RefreshAssetTree();
					}
				}
				ImGui::EndPopup();
			}

			if (ImGui::BeginPopup("Rename Item")) {
				static char renameBuffer[128];
				ImGui::InputText("New Name", renameBuffer, sizeof(renameBuffer));
				if (ImGui::Button("Rename")) {
					std::filesystem::path oldPath = _currentDirectory / entry.path.filename();
					std::filesystem::path newPath = _currentDirectory / renameBuffer;
					if (std::filesystem::exists(oldPath) && !std::filesystem::exists(newPath)) {
						std::filesystem::rename(oldPath, newPath);
						RefreshAssetTree();
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			if (ImGui::BeginDragDropSource())
			{
				AssetHandle handle = entry.handle;
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &handle, sizeof(AssetHandle));
				ImGui::Text("%s", entry.path.stem().string().c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (entry.isDirectory)
				{
					_currentDirectory = entry.path;
					RefreshAssetTree();
				}
			}

			ImGui::TextWrapped(item.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		if (ImGui::BeginPopupContextWindow("EmptySpaceContext", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem("Create Script")) {
				std::filesystem::path scriptPath = _currentDirectory / "NewScript.cs";
				if (!std::filesystem::exists(scriptPath)) {
					std::ofstream(scriptPath.string());
					RefreshAssetTree();
				}
			}

			if (ImGui::MenuItem("Create Folder")) {
				ImGui::OpenPopup("Create New Folder");
				RefreshAssetTree();
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Create New Folder")) {
			static char folderName[128] = "NewFolder";
			ImGui::InputText("Folder Name", folderName, sizeof(folderName));
			if (ImGui::Button("Create")) {
				std::filesystem::path newFolder = _currentDirectory / folderName;
				if (!std::filesystem::exists(newFolder)) {
					std::filesystem::create_directory(newFolder);
					RefreshAssetTree();
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void ContentBrowserPanel::RefreshAssetTree()
	{
		_directoryEntries.clear();

		for (const auto& entry : std::filesystem::directory_iterator(_currentDirectory))
		{
			DirectoryEntry dirEntry;
			dirEntry.path = entry.path();
			dirEntry.isDirectory = entry.is_directory();

			if (!dirEntry.isDirectory) {
				dirEntry.handle = AssetManager::GetMetadata(dirEntry.path).handle;
			}

			_directoryEntries.push_back(dirEntry);
		}
	}
}