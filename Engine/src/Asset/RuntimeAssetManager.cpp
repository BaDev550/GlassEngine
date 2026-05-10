#include "Asset/AssetManager.h"

namespace ge {
	RuntimeAssetManager::RuntimeAssetManager(const filesystem::Path& assetPakPath, const filesystem::Path& assetManifestPath)
	{
		LoadManifest(assetManifestPath);
		LoadAssetsFromPak(assetPakPath);
	}

	mem::Ref<Asset> RuntimeAssetManager::GetOrImportAsset(filesystem::Path sourcePath, ImportAssetData asset, filesystem::Path targetPath, AssetHandle handle)
	{
		if (handle != GE_INVALID_ASSET_HANDLE) {
			return GetAsset(handle);
		}
		else {
			if (_assetManifest.contains(GetPathWithoutExtension(sourcePath).c_str())) {
				AssetHandle manifestHandle = _assetManifest[GetPathWithoutExtension(sourcePath).c_str()];
				return GetAsset(manifestHandle);
			}
		}
		return nullptr;
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
			GEVector<uint8_t> buffer(packedAsset.size);
			_pakFileReader.SetStreamPosition(packedAsset.offset);
			_pakFileReader.ReadData(reinterpret_cast<char*>(buffer.data()), packedAsset.size);
			mem::Ref<Asset> asset = _importer->DeserializeFromFile(mtd, buffer);
			if (asset) {
				asset->_assetHandle = handle;
				_loadedAssets[handle] = asset;
				return asset;
			}
		}
		return nullptr;
	}

	AssetMetadata RuntimeAssetManager::GetMetadata(AssetHandle handle)
	{
		if (!AssetInRegistry(handle))
			return AssetMetadata();

		AssetMetadata mtd;
		mtd.handle = _assetRegistry[handle].handle;
		return mtd;
	}

	AssetMetadata RuntimeAssetManager::GetMetadata(const filesystem::Path& path)
	{
		if (!_assetManifest.contains(path.string()))
			return AssetMetadata();

		AssetMetadata mtd;
		mtd.handle = _assetManifest[path.string()];
		return mtd;
	}

	void RuntimeAssetManager::LoadManifest(const filesystem::Path& manifestPath)
	{
		_manifestFileReader.OpenStream(manifestPath);
		if (!_manifestFileReader.IsStreamGood()) {
			GE_CORE_CRITICAL("Failed to open manifest file");
			return;
		}

		size_t assetCount = 0;
		_manifestFileReader.ReadData(reinterpret_cast<char*>(&assetCount), sizeof(size_t));
		for (size_t i = 0; i < assetCount; i++) {
			uint32_t pathLength = 0;
			AssetHandle handle = GE_INVALID_ASSET_HANDLE;
			_manifestFileReader.ReadData(reinterpret_cast<char*>(&pathLength), sizeof(uint32_t));
			GEString path(pathLength, '\0');
			_manifestFileReader.ReadData(path.data(), pathLength);
			_manifestFileReader.ReadData(reinterpret_cast<char*>(&handle), sizeof(AssetHandle));
			_assetManifest[path] = handle;
		}
	}

	void RuntimeAssetManager::LoadAssetsFromPak(const filesystem::Path& assetPakPath)
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
}