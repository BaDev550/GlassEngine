#pragma once
#include "Engine.h"
#include "Asset.h"
#include "AssetMetadata.h"
#include "AssetImporter.h"
#include "Serialization/FileSerializer.h"

#include <map>
#include <unordered_set>

namespace ge {
	using AssetRegistry = GEUnorderedMap<AssetHandle, AssetMetadata>;
	using AssetMap = GEMap<AssetHandle, mem::Ref<Asset>>;
	using AssetManifest = GEUnorderedMap<GEString, AssetHandle>;
	using PackedAssetMap = GEMap<AssetHandle, PackedAsset>;
	using DependencyMap = GEUnorderedMap<AssetHandle, std::unordered_set<AssetHandle>>;

	class AssetManagerBase : public mem::RefCounted {
	public:
		AssetManagerBase() { _importer = mem::CreateScope<AssetImporter>(); }
		virtual ~AssetManagerBase() = default;
		AssetManagerBase(AssetManagerBase&) = delete;
		AssetManagerBase& operator=(AssetManagerBase&) = delete;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) = 0;
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(filesystem::Path sourcePath, ImportAssetData asset = ImportAssetData(), filesystem::Path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) = 0;
		[[nodiscard]] virtual AssetMap GetLoadedAssets() const = 0;
		[[nodiscard]] virtual AssetMetadata GetMetadata(AssetHandle handle) = 0;
		[[nodiscard]] virtual AssetMetadata GetMetadata(const filesystem::Path& path) = 0;
		[[nodiscard]] virtual bool AssetInRegistry(AssetHandle handle) = 0;
		filesystem::Path TryToGetGassetFile(filesystem::Path sourcePath);
	protected:
		mem::Scope<AssetImporter> _importer;
	};

	class EditorAssetManager : public AssetManagerBase {
	public:
		EditorAssetManager(const filesystem::Path& assetRegistry);
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(filesystem::Path sourcePath, ImportAssetData asset = ImportAssetData(), filesystem::Path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) override;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
		[[nodiscard]] virtual AssetMap GetLoadedAssets() const override { return _loadedAssets; }
		[[nodiscard]] virtual AssetMetadata GetMetadata(AssetHandle handle) override;
		[[nodiscard]] virtual AssetMetadata GetMetadata(const filesystem::Path& path) override;
		[[nodiscard]] virtual bool AssetInRegistry(AssetHandle handle) override;
		[[nodiscard]] AssetHandle ImportAsset(const ImportAssetData& asset, filesystem::Path sourcePath, filesystem::Path targetPath = "");
		[[nodiscard]] mem::Ref<Asset> LoadAssetFromFile(AssetHandle handle);
		[[nodiscard]] AssetMap GetLoadedAssetsWithType(AssetType type);
		void RemoveAsset(AssetHandle handle);

		void SetAssetRegistryPath(const filesystem::Path& path) { _assetRegistryPath = path; }
		void CompileIntoPakFile(const filesystem::Path& outPath);
		void CompileIntoManifest(const filesystem::Path& outPath);
		void CookAssets(const filesystem::Path& outPath);
		void DeleteAllGAssetFiles();

		[[nodiscard]] AssetHandle CreateAsset(const filesystem::Path& targetPath, mem::Ref<Asset> asset);

		void RegisterDependency(AssetHandle handle, AssetHandle dependency);
		void UpdateDependencies(AssetHandle handle);
		void OnAssetDeleted(AssetHandle handle);
	private:
		bool AssetLoaded(AssetHandle handle);

		AssetRegistry _assetRegistry;
		AssetMap _loadedAssets;
		DependencyMap _assetDependencies;

		void SaveAssetRegistry();
		bool LoadAssetRegistry();
		bool TryToRebuildRegistry();
		filesystem::Path _assetRegistryPath = "assetregistry.ge";
	};

	class RuntimeAssetManager : public AssetManagerBase {
	public:
		RuntimeAssetManager(const filesystem::Path& assetPakPath, const filesystem::Path& assetManifestPath);
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(filesystem::Path sourcePath, ImportAssetData asset = ImportAssetData(), filesystem::Path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) override;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
		[[nodiscard]] virtual AssetMap GetLoadedAssets() const override { return _loadedAssets; }
		[[nodiscard]] virtual AssetMetadata GetMetadata(AssetHandle handle) override;
		[[nodiscard]] virtual AssetMetadata GetMetadata(const filesystem::Path& path) override;
		virtual bool AssetInRegistry(AssetHandle handle) override;

		void LoadManifest(const filesystem::Path& manifestPath);
		void LoadAssetsFromPak(const filesystem::Path& assetPakPath);
	private:
		bool AssetLoaded(AssetHandle handle);
		PackedAssetMap _assetRegistry;
		AssetMap _loadedAssets;
		AssetManifest _assetManifest;
		file::Reader _manifestFileReader;
		file::Reader _pakFileReader;
	};

	inline GEString GetPathWithoutExtension(const filesystem::Path& path) {
		return (path.GetParentPath() / path.GetFileName()).string();
	}

	class Project;
	class AssetManager {
	public:
		static void Init(Project* project);
		static void Destroy();

		[[nodiscard]] static AssetMetadata GetMetadata(AssetHandle handle);
		[[nodiscard]] static AssetMetadata GetMetadata(const filesystem::Path& path);
		[[nodiscard]] static bool AssetInRegistry(AssetHandle handle);

		template<typename T>
		static mem::Ref<T> GetAsset(AssetHandle handle) {
			return _assetManager->GetAsset(handle).Cast<T>();
		}
		template<typename T>
		static mem::Ref<T> GetOrImportAsset(filesystem::Path sourcePath, filesystem::Path targetPath = "", ImportAssetData asset = ImportAssetData(), AssetHandle handle = GE_INVALID_ASSET_HANDLE) {
			return _assetManager->GetOrImportAsset(sourcePath, asset, targetPath, handle).Cast<T>();
		}

		static void Editor_RegisterDependency(AssetHandle handle, AssetHandle dependency);
		static void Editor_CompileIntoPakFile(const filesystem::Path& outPath);
		static void Editor_CompileIntoManifest(const filesystem::Path& outPath);
		static void Editor_CookAssets(const filesystem::Path& outPath);
		static void Editor_OnAssetDeleted(AssetHandle handle);
		[[nodiscard]] static AssetMap Editor_GetLoadedAssetsWithType(AssetType type);
		[[nodiscard]] static AssetHandle Editor_ImportAsset(const ImportAssetData& asset, filesystem::Path sourcePath, filesystem::Path targetPath = "");
		[[nodiscard]] static AssetHandle Editor_CreateAsset(const filesystem::Path& targetPath, mem::Ref<Asset> asset);
	private:
		static mem::Ref<AssetManagerBase> _assetManager;
		static EditorAssetManager* _editorAssetManagerInstance;
		static RuntimeAssetManager* _runtimeAssetManagerInstance;
	};
}