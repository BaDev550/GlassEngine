#pragma once
#include "Asset.h"
#include "AssetMetadata.h"
#include "AssetImporter.h"
#include "GlassEngine/Serialization/FileSerializer.h"
#include <map>

namespace ge {
	using AssetRegistry = GEUnorderedMap<AssetHandle, AssetMetadata>;
	using AssetMap = GEMap<AssetHandle, mem::Ref<Asset>>;
	using AssetManifest = GEUnorderedMap<GEString, AssetHandle>;
	using PackedAssetMap = GEMap<AssetHandle, PackedAsset>;
	class AssetManager {
	public:
		AssetManager() { _importer = mem::CreateScope<AssetImporter>(); }
		virtual ~AssetManager() = default;
		AssetManager(AssetManager&) = delete;
		AssetManager& operator=(AssetManager&) = delete;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) = 0;
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(std::filesystem::path sourcePath, ImportAssetData asset = ImportAssetData(), std::filesystem::path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) = 0;
	protected:
		mem::Scope<AssetImporter> _importer;
	};

	class EditorAssetManager : public AssetManager {
	public:
		EditorAssetManager();
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(std::filesystem::path sourcePath, ImportAssetData asset = ImportAssetData(), std::filesystem::path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) override;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
		[[nodiscard]] AssetHandle ImportAsset(const ImportAssetData& asset, std::filesystem::path sourcePath, std::filesystem::path targetPath = "");
		[[nodiscard]] AssetMap GetLoadedAssets() const { return _loadedAssets; }
		[[nodiscard]] mem::Ref<Asset> LoadAssetFromFile(AssetHandle handle);
		[[nodiscard]] AssetHandle CreateAsset(const std::filesystem::path& targetPath, mem::Ref<Asset> asset);

		void ImportAssetAsync(const ImportAssetData& asset, std::function<void(AssetHandle)> loadedFunc, std::filesystem::path sourcePath, std::filesystem::path targetPath = "");
		void CompileIntoPakFile(const std::filesystem::path& outPath);
		void CompileIntoManifest(const std::filesystem::path& outPath);
		void CookAssets(const std::filesystem::path& outPath);
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
		RuntimeAssetManager(const std::filesystem::path& assetPakPath, const std::filesystem::path& assetManifestPath);
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(std::filesystem::path sourcePath, ImportAssetData asset = ImportAssetData(), std::filesystem::path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) override;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;

		void LoadManifest(const std::filesystem::path& manifestPath);
		void LoadAssetsFromPak(const std::filesystem::path& assetPakPath);
	private:
		bool AssetLoaded(AssetHandle handle);
		bool AssetInRegistry(AssetHandle handle);
		PackedAssetMap _assetRegistry;
		AssetMap _loadedAssets;
		AssetManifest _assetManifest;
		file::Reader _manifestFileReader;
		file::Reader _pakFileReader;
	};

	inline std::string GetPathWithoutExtension(const std::filesystem::path& path) {
		return (path.parent_path() / path.stem()).generic_string();
	}
}