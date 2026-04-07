#pragma once
#include "Asset.h"
#include "AssetMetadata.h"
#include "AssetImporter.h"
#include <map>

namespace ge {
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
	using AssetMap = std::map<AssetHandle, mem::Ref<Asset>>;
	class AssetManager {
	public:
		AssetManager() { _importer = mem::CreateScope<AssetImporter>(); }
		virtual ~AssetManager() = default;
		AssetManager(AssetManager&) = delete;
		AssetManager& operator=(AssetManager&) = delete;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) = 0; // Gets the loaded asset from handle
	protected:
		mem::Scope<AssetImporter> _importer;
	};

	class EditorAssetManager : public AssetManager {
	public:
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
		AssetHandle ImportAsset(std::filesystem::path sourcePath, std::filesystem::path targetPath = "");
		mem::Ref<Asset> LoadAssetFromFile(AssetHandle handle);
	private:
		bool AssetInRegistry(AssetHandle handle);
		bool AssetLoaded(AssetHandle handle);
		AssetMetadata GetMetadata(AssetHandle handle);
		AssetMetadata GetMetadata(const std::filesystem::path& path);

		AssetRegistry _assetRegistry;
		AssetMap _loadedAssets;
	};

	class RuntimeAssetManager : public AssetManager {
	public:
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) {};
	};
}