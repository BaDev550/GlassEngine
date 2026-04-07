#include "gepch.h"
#include "AssetManager.h"

namespace ge {
	RuntimeAssetManager::RuntimeAssetManager(const std::filesystem::path& assetPakPath)
	{
		_pakFileReader.OpenStream(assetPakPath);
		if (!_pakFileReader.IsStreamGood()) {
			GE_CORE_CRITICAL("Failed to open .pak file");
			return;
		}

		size_t assetCount = 0;
		_pakFileReader.ReadData(reinterpret_cast<char*>(&assetCount), sizeof(size_t));
		for (size_t i = 0; i < assetCount; i++) {
			AssetHandle handle;
			PackedAsset packedAsset;

			_pakFileReader.ReadData(reinterpret_cast<char*>(&handle), sizeof(AssetHandle));
			_pakFileReader.ReadData(reinterpret_cast<char*>(&packedAsset.offset), sizeof(uint64_t));
			_pakFileReader.ReadData(reinterpret_cast<char*>(&packedAsset.size), sizeof(uint64_t));
			_pakFileReader.ReadData(reinterpret_cast<char*>(&packedAsset.type), sizeof(AssetType));
			_assetRegistry[handle] = packedAsset;
		}
	}

	bool RuntimeAssetManager::AssetInRegistry(AssetHandle handle) { return _assetRegistry.contains(handle); }
	bool RuntimeAssetManager::AssetLoaded(AssetHandle handle) { return _loadedAssets.contains(handle); }

	mem::Ref<Asset> RuntimeAssetManager::GetAsset(AssetHandle handle)
	{
		if (AssetLoaded(handle))
			return _loadedAssets.at(handle);

		if (AssetInRegistry(handle)) {
			const auto& packedAsset = _assetRegistry[handle];
			AssetMetadata mtd;
			mtd.handle = handle;
			mtd.type = packedAsset.type;
			std::vector<uint8_t> buffer(packedAsset.size);
			_pakFileReader.SetStreamPosition(packedAsset.offset);
			_pakFileReader.ReadData(reinterpret_cast<char*>(buffer.data()), packedAsset.size);
			mem::Ref<Asset> asset = _importer->DeserializeFromFile(mtd, buffer);
			if (asset) {
				_loadedAssets[handle] = asset;
				return asset;
			}
		}
		return nullptr;
	}
}