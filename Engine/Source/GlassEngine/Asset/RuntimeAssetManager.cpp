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
			_loadedAssets[handle] = packedAsset;
		}
	}

	mem::Ref<Asset> RuntimeAssetManager::GetAsset(AssetHandle handle)
	{
		return mem::Ref<Asset>();
	}
}