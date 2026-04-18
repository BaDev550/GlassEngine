#pragma once
#include "Asset.h"
#include "AssetMetadata.h"
#include "AssetImporter.h"
#include "GlassEngine/Serialization/FileSerializer.h"
#include <map>

namespace ge {
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
	using AssetMap = std::map<AssetHandle, mem::Ref<Asset>>;
	using PackedAssetMap = std::map<AssetHandle, PackedAsset>;
	class AssetManager {
	public:
		AssetManager() { _importer = mem::CreateScope<AssetImporter>(); }
		virtual ~AssetManager() = default;
		AssetManager(AssetManager&) = delete;
		AssetManager& operator=(AssetManager&) = delete;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) = 0;
	protected:
		mem::Scope<AssetImporter> _importer;
	};

	class EditorAssetManager : public AssetManager {
	public:
		EditorAssetManager();
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
		AssetHandle ImportAsset(const ImportAssetData& asset, std::filesystem::path sourcePath, std::filesystem::path targetPath = "");
		AssetMap GetLoadedAssets() const { return _loadedAssets; }
		void ImportAssetAsync(const ImportAssetData& asset, std::function<void(AssetHandle)> loadedFunc, std::filesystem::path sourcePath, std::filesystem::path targetPath = "");
		mem::Ref<Asset> LoadAssetFromFile(AssetHandle handle);
		void CompileIntoPakFile(const std::filesystem::path& outPath);
	private:
		bool AssetInRegistry(AssetHandle handle);
		bool AssetLoaded(AssetHandle handle);
		AssetMetadata GetMetadata(AssetHandle handle);
		AssetMetadata GetMetadata(const std::filesystem::path& path);

		AssetRegistry _assetRegistry;
		AssetMap _loadedAssets;

		void SaveAssetRegistry();
		void LoadAssetRegistry();
		const std::filesystem::path _assetRegistryPath = "assetregistry.ge";
	};

	class RuntimeAssetManager : public AssetManager {
	public:
		RuntimeAssetManager(const std::filesystem::path& assetPakPath);
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
	private:
		bool AssetLoaded(AssetHandle handle);
		bool AssetInRegistry(AssetHandle handle);
		PackedAssetMap _assetRegistry;
		AssetMap _loadedAssets;
		file::Reader _pakFileReader;
	};
}