#pragma once
#include "Asset.h"
#include "AssetMetadata.h"
#include "AssetImporter.h"
#include "GlassEngine/Serialization/FileSerializer.h"
#include <map>

namespace ge {
	using AssetRegistry = GEUnorderedMap<AssetHandle, AssetMetadata, mem::AssetAllocTag>;
	using AssetMap = GEMap<AssetHandle, mem::Ref<Asset>, mem::AssetAllocTag>;
	using AssetManifest = GEUnorderedMap<GEString, AssetHandle, mem::AssetAllocTag>;
	using PackedAssetMap = GEMap<AssetHandle, PackedAsset, mem::AssetAllocTag>;
	class AssetManagerBase : public mem::RefCounted {
	public:
		AssetManagerBase() { _importer = mem::CreateScope<AssetImporter>(); }
		virtual ~AssetManagerBase() = default;
		AssetManagerBase(AssetManagerBase&) = delete;
		AssetManagerBase& operator=(AssetManagerBase&) = delete;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) = 0;
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(std::filesystem::path sourcePath, ImportAssetData asset = ImportAssetData(), std::filesystem::path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) = 0;
		[[nodiscard]] virtual AssetMap GetLoadedAssets() const = 0;
	protected:
		mem::Scope<AssetImporter> _importer;
	};

	class EditorAssetManager : public AssetManagerBase {
	public:
		EditorAssetManager();
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(std::filesystem::path sourcePath, ImportAssetData asset = ImportAssetData(), std::filesystem::path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) override;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
		[[nodiscard]] virtual AssetMap GetLoadedAssets() const override { return _loadedAssets; }
		[[nodiscard]] AssetHandle ImportAsset(const ImportAssetData& asset, std::filesystem::path sourcePath, std::filesystem::path targetPath = "");
		[[nodiscard]] mem::Ref<Asset> LoadAssetFromFile(AssetHandle handle);
		[[nodiscard]] AssetHandle CreateAsset(const std::filesystem::path& targetPath, mem::Ref<Asset> asset);

		void SetAssetRegistryPath(const std::filesystem::path& path) { _assetRegistryPath = path; }
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
		std::filesystem::path _assetRegistryPath = "assetregistry.ge";
	};

	class RuntimeAssetManager : public AssetManagerBase {
	public:
		RuntimeAssetManager(const std::filesystem::path& assetPakPath, const std::filesystem::path& assetManifestPath);
		[[nodiscard]] virtual mem::Ref<Asset> GetOrImportAsset(std::filesystem::path sourcePath, ImportAssetData asset = ImportAssetData(), std::filesystem::path targetPath = "", AssetHandle handle = GE_INVALID_ASSET_HANDLE) override;
		[[nodiscard]] virtual mem::Ref<Asset> GetAsset(AssetHandle handle) override;
		[[nodiscard]] virtual AssetMap GetLoadedAssets() const override { return _loadedAssets; }

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

	inline GEString GetPathWithoutExtension(const std::filesystem::path& path) {
		return (path.parent_path() / path.stem()).generic_string();
	}

	class Project;
	class AssetManager {
	public:
		static void Init(Project* project);
		static void Destroy();

		template<typename T>
		static mem::Ref<T> GetAsset(AssetHandle handle) {
			return _assetManager->GetAsset(handle).Cast<T>();
		}
		template<typename T>
		static mem::Ref<T> GetOrImportAsset(std::filesystem::path sourcePath, std::filesystem::path targetPath = "", ImportAssetData asset = ImportAssetData(), AssetHandle handle = GE_INVALID_ASSET_HANDLE) {
			return _assetManager->GetOrImportAsset(sourcePath, asset, targetPath, handle).Cast<T>();
		}

		static void Editor_CompileIntoPakFile(const std::filesystem::path& outPath);
		static void Editor_CompileIntoManifest(const std::filesystem::path& outPath);
		static void Editor_CookAssets(const std::filesystem::path& outPath);
		[[nodiscard]] static AssetHandle Editor_ImportAsset(const ImportAssetData& asset, std::filesystem::path sourcePath, std::filesystem::path targetPath = "");
		[[nodiscard]] static AssetHandle Editor_CreateAsset(const std::filesystem::path& targetPath, mem::Ref<Asset> asset);
	private:
		static mem::Ref<AssetManagerBase> _assetManager;
		static EditorAssetManager* _editorAssetManagerInstance;
		static RuntimeAssetManager* _runtimeAssetManagerInstance;
	};
}