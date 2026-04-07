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
			if (mtd.IsLoaded()) { GE_CORE_CRITICAL("Well this doesnt supposed to be happening") } // Error this shoud be always return false
			else {
				return LoadAssetFromFile(handle);
			}
		}
		return nullptr;
	}

	mem::Ref<Asset> EditorAssetManager::LoadAssetFromFile(AssetHandle handle) {
		if (!AssetInRegistry(handle)) {
			GE_CORE_ERROR("Failed to find {} in registry", handle);
			return nullptr;
		}
		AssetMetadata mtd = GetMetadata(handle);
		file::Reader fileReader(mtd.path);
		if (fileReader.IsStreamGood()) {
			char magic[4];
			fileReader.SetStreamPosition(0);
			fileReader.ReadData(magic, 4);
			mem::Ref<Asset> asset = nullptr;
			if (strncmp(magic, MODEL_MAGIC, 4) == 0) { // TODO (badev): make a importer class to handle this ones
				// Load model static or skeletal mesh
			}
		}
		return mem::Ref<Asset>();
	}

	bool EditorAssetManager::AssetInRegistry(AssetHandle handle) { return _assetRegistry.contains(handle); }
	bool EditorAssetManager::AssetLoaded(AssetHandle handle) { return _loadedAssets.contains(handle); }
	AssetMetadata EditorAssetManager::GetMetadata(AssetHandle handle)
	{
		auto it = _assetRegistry.find(handle);
		if (it != _assetRegistry.end())
			return it->second;
		return AssetMetadata();
	}
}