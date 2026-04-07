#include "gepch.h"
#include "AssetManager.h"
#include "GlassEngine/Asset/AssetExtensions.h"
#include "GlassEngine/Serialization/FileSerializer.h"

namespace ge {
	mem::Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) {
		if (AssetLoaded(handle))
			return _loadedAssets.at(handle);

		AssetMetadata mtd = GetMetadata(handle);
		if (mtd.IsValid()) {
			if (mtd.IsLoaded()) { GE_CORE_CRITICAL("Well this doesnt supposed to be happening"); } // Error this shoud be always return false
			else {
				return LoadAssetFromFile(handle);
			}
		}
		return nullptr;
	}

	AssetHandle EditorAssetManager::ImportAsset(std::filesystem::path sourcePath, std::filesystem::path targetPath)
	{
		if (!std::filesystem::exists(sourcePath)) {
			GE_CORE_ERROR("Cannot import asset. Source file does not exist: {}", sourcePath.string());
			return GE_INVALID_ASSET_HANDLE;
		}

		if (targetPath.empty())
			targetPath = sourcePath;
		AssetHandle handle = AssetHandle();
		AssetType compiledType = _importer->ImportToGAsset(sourcePath, targetPath);
		if (compiledType != AssetType::Unknown) {
			AssetMetadata mtd;
			mtd.handle = handle;
			mtd.path = targetPath;
			mtd.type = compiledType;
			mtd.state = AssetLoadingState::Loading;
			_assetRegistry[handle] = mtd;
			GE_CORE_INFO("Asset imported from: {} to: {}", sourcePath.string(), targetPath.string());
			return handle;
		}
		return GE_INVALID_ASSET_HANDLE;
	}

	mem::Ref<Asset> EditorAssetManager::LoadAssetFromFile(AssetHandle handle) {
		if (!AssetInRegistry(handle)) {
			//GE_CORE_ERROR("Failed to find {} in registry", handle);
			return nullptr;
		}
		AssetMetadata& mtd = _assetRegistry[handle];
		file::Reader fileReader(mtd.path);
		if (!fileReader.IsStreamGood()) {
			GE_CORE_ERROR("Failed to open asset file: {}", mtd.path.string());
			return nullptr;
		}

		mem::Ref<Asset> asset = nullptr;
		if (asset = _importer->DeserializeFromFile(mtd)) {
			asset->_assetHandle = handle;
			_loadedAssets[handle] = asset;
			mtd.state = AssetLoadingState::Loaded;
			return asset;
		}
		else {
			GE_CORE_ERROR("Failed to deserialize asset: {}", mtd.path.string());
		}
		return nullptr;
	}

	bool EditorAssetManager::AssetInRegistry(AssetHandle handle) { return _assetRegistry.contains(handle); }
	bool EditorAssetManager::AssetLoaded(AssetHandle handle) { return _loadedAssets.contains(handle); }
	AssetMetadata EditorAssetManager::GetMetadata(AssetHandle handle)
	{
		if (_assetRegistry.contains(handle))
			return _assetRegistry[handle];
		return AssetMetadata();
	}

	AssetMetadata EditorAssetManager::GetMetadata(const std::filesystem::path& path)
	{
		for (auto [handle, mtd] : _assetRegistry)
			if (mtd.path == path)
				return mtd;
		return AssetMetadata();
	}
}