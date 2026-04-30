#pragma once
#include <filesystem>
#include <map>
#include <set>

#include "GlassEngine/Renderer/Texture.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Editor/EditorPanel.h"

// Forked from https://github.com/BaDev550/Tourqe-Advanced-Game-Engine
namespace ge::editor {
	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel();

		void Init();
		virtual void Draw() override;
	private:
		void RefreshAssetTree();
	private:
		std::filesystem::path _currentDirectory;
		std::filesystem::path _baseDirectory;

		struct DirectoryEntry {
			std::filesystem::path path;
			bool isDirectory;
			AssetHandle handle = 0;
		};
		GEVector<DirectoryEntry> _directoryEntries;

		GEVector<mem::Ref<renderer::Texture2D>> _icons;
		ImTextureID _directoryIconTextureId;
		ImTextureID _defaultFileIconTextureId;
		ImTextureID _staticMeshFileIconTextureId;
		ImTextureID _textureFileIconTextureId;
		ImTextureID _materialFileIconTextureId;

		float _padding = 19.0f;
		float _thumbnailSize = 66.0f;
	};
}