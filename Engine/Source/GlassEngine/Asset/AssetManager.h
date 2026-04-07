#pragma once
#include "Asset.h"
#include "AssetMetadata.h"
#include <map>

namespace ge {
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
	using AssetMap = std::map<AssetHandle, mem::Ref<Asset>>;
	class AssetManager {
	public:
		virtual ~AssetManager() = default;
		AssetManager(AssetManager&) = delete;
		AssetManager& operator=(AssetManager&) = delete;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) = 0; // Gets the loaded asset from handle
	};

	class EditorAssetManager : public AssetManager {
	public:
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
		mem::Ref<Asset> LoadAssetFromFile(AssetHandle handle);
	private:
		bool AssetInRegistry(AssetHandle handle);
		bool AssetLoaded(AssetHandle handle);
		AssetMetadata GetMetadata(AssetHandle handle);

		AssetRegistry _assetRegistry;
		AssetMap _loadedAssets;
	};

	class RuntimeAssetManager : public AssetManager {
	public:
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) {};
	};
}