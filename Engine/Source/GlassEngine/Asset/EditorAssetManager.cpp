#include "gepch.h"
#include "AssetManager.h"
#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Asset/AssetExtensions.h"

namespace ge {
	EditorAssetManager::EditorAssetManager()
	{
		LoadAssetRegistry();

		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_EDITOR_ASSETMANAGER_CATAGORY, "cookAssets", [this](const GEVector<GEString>& args) { CookAssets(args[0]); }, "cookAssets <outputPakFile>");
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_EDITOR_ASSETMANAGER_CATAGORY, "compileAssetsToPAK", [this](const GEVector<GEString>& args) { CompileIntoPakFile(args[0]); }, "compileAssetsToPAK <outputPakFile>");
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_EDITOR_ASSETMANAGER_CATAGORY, "compileAssetsToManifest", [this](const GEVector<GEString>& args) { CompileIntoManifest(args[0]); }, "compileAssetsToManifest <outputPakFile>");
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_EDITOR_ASSETMANAGER_CATAGORY, "loadAsset", [this](const GEVector<GEString>& args) { 
			AssetHandle handle = ImportAsset(ImportAssetData(), args[0]); 
			GE_CORE_INFO("Asset Loaded: {} Handle: {}", args[0], handle.ToString());
			}, "loadAsset <assetPath>");
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_EDITOR_ASSETMANAGER_CATAGORY, "clearRegistry", [this](const GEVector<GEString>& args) {
			_assetRegistry.clear();
			SaveAssetRegistry();
			GE_CORE_INFO("Asset registry cleared");
			});
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_EDITOR_ASSETMANAGER_CATAGORY, "printRegistry", [this](const GEVector<GEString>& args) {
			GE_CORE_INFO("Asset Registry:");
			for (const auto& [handle, mtd] : _assetRegistry) {
				GE_CORE_INFO("- Handle: {}, Path: {}, Type: {}, State: {}", handle.ToString(), mtd.path.string(), AssetTypeToString(mtd.type), AssetLoadingStateToString(mtd.state));
			}
			});
		GE_ADD_CONSOLE_COMMAND(GE_CONSOLE_EDITOR_ASSETMANAGER_CATAGORY, "printLoadedAssets", [this](const GEVector<GEString>& args) {
			GE_CORE_INFO("Loaded Assets:");
			for (const auto& [handle, asset] : _loadedAssets) {
				GE_CORE_INFO("- Handle: {}, Type: {}", handle.ToString(), AssetTypeToString(asset->GetAssetType()));
			}
			});
	}

	mem::Ref<Asset> EditorAssetManager::GetOrImportAsset(std::filesystem::path sourcePath, ImportAssetData asset, std::filesystem::path targetPath, AssetHandle handle) {
		if (handle != GE_INVALID_ASSET_HANDLE) {
			if (AssetLoaded(handle))
				return _loadedAssets.at(handle);
			else
				return LoadAssetFromFile(handle);
		}
		else if (!sourcePath.empty()) {
			auto mtd = GetMetadata(sourcePath);
			if (mtd.IsValid()) {
				if (mtd.IsLoaded())
					return _loadedAssets.at(mtd.handle);
				else
					return LoadAssetFromFile(mtd.handle);
			}
			else {
				AssetHandle newHandle = ImportAsset(asset, sourcePath, targetPath);
				if (newHandle != GE_INVALID_ASSET_HANDLE)
					return LoadAssetFromFile(newHandle);
			}
		}
		return nullptr;
	}

	mem::Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) {
		if (AssetLoaded(handle))
			return _loadedAssets.at(handle);

		AssetMetadata mtd = GetMetadata(handle);
		if (mtd.IsValid()) {
			if (mtd.IsLoaded()) { GE_CORE_CRITICAL("Well this doesnt supposed to be happening"); } // Error this shoud be always return false
			else {
				return LoadAssetFromFile(handle);
			}
		}
		return nullptr;
	}

	void EditorAssetManager::ImportAssetAsync(const ImportAssetData& asset, std::function<void(AssetHandle)> loadedFunc, std::filesystem::path sourcePath, std::filesystem::path targetPath) {
		Application::Get()->GetThreadManager().Enqueue([this, asset, sourcePath, targetPath, loadedFunc]() {
			AssetHandle result = ImportAsset(asset, sourcePath, targetPath);
			if (loadedFunc) {
				loadedFunc(result);
				GE_CORE_INFO("Asset Loaded: {}", sourcePath.string());
			}
			});
	}

	AssetHandle EditorAssetManager::ImportAsset(const ImportAssetData& asset, std::filesystem::path sourcePath, std::filesystem::path targetPath)
	{
		if (!std::filesystem::exists(sourcePath)) {
			GE_CORE_ERROR("Cannot import asset. Source file does not exist: {}", sourcePath.string());
			return GE_INVALID_ASSET_HANDLE;
		}

		if (targetPath.empty())
			targetPath = sourcePath;

		AssetHandle handle = AssetHandle();
		AssetType compiledType = _importer->ImportToGAsset(asset, sourcePath, targetPath);
		GE_CORE_INFO("Getting Asset from: {}", sourcePath.string());

		auto existingMtd = GetMetadata(targetPath);
		if (existingMtd.IsValid())
			return existingMtd.handle;
		
		if (compiledType != AssetType::Unknown) {
			AssetMetadata mtd;
			mtd.handle = handle;
			mtd.path = targetPath;
			mtd.type = compiledType;
			mtd.state = AssetLoadingState::Loading;
			_assetRegistry[handle] = mtd;
			SaveAssetRegistry();
			GE_CORE_INFO("Asset imported from: {} to: {}", sourcePath.string(), targetPath.string());
			return handle;
		}
		return GE_INVALID_ASSET_HANDLE;
	}

	mem::Ref<Asset> EditorAssetManager::LoadAssetFromFile(AssetHandle handle) {
		if (!AssetInRegistry(handle)) {
			GE_CORE_ERROR("Failed to find {} in registry", handle.ToString());
			return nullptr;
		}
		AssetMetadata& mtd = _assetRegistry[handle];
		file::Reader fileReader(mtd.path);
		if (!fileReader.IsStreamGood()) {
			GE_CORE_ERROR("Failed to open asset file: {}", mtd.path.string());
			return nullptr;
		}
		GE_PROFILE_SCOPE(("EditorAssetManager::LoadAssetFromFile " + mtd.path.string()).c_str());

		mem::Ref<Asset> asset = nullptr;
		if (asset = _importer->DeserializeFromFile(mtd)) {
			asset->_assetHandle = handle;
			_loadedAssets[handle] = asset;
			mtd.state = AssetLoadingState::Loaded;
			return asset;
		}
		else {
			GE_CORE_ERROR("Failed to deserialize asset: {}", mtd.path.string());
		}
		return nullptr;
	}

	void EditorAssetManager::CompileIntoPakFile(const std::filesystem::path& outPath)
	{
		GE_PROFILE_SCOPE("EditorAssetManager::CompileIntoPakFile");

		file::Writer out(outPath);
		size_t assetCount = _assetRegistry.size();
		out.WriteData(reinterpret_cast<const char*>(&assetCount), sizeof(size_t));

		uint64_t startIndex = out.GetStreamPosition();
		for (auto& [handle, mtd] : _assetRegistry) { // Write the data's asset into pak
			uint64_t zero = 0;
			out.WriteData(reinterpret_cast<const char*>(&handle),   sizeof(AssetHandle));
			out.WriteData(reinterpret_cast<const char*>(&zero),     sizeof(uint64_t));
			out.WriteData(reinterpret_cast<const char*>(&zero),     sizeof(uint64_t));
			out.WriteData(reinterpret_cast<const char*>(&mtd.type), sizeof(AssetType));
		}
		struct TempEntry { uint64_t offset; uint64_t size; };
		std::map<AssetHandle, TempEntry> finalEntries;

		for (auto& [handle, mtd] : _assetRegistry) { // Open all the files in registry and write into .pak
			file::Reader assetReader(mtd.path, std::ios::binary | std::ios::ate);
			uint64_t size = assetReader.GetStreamPosition();
			uint64_t offset = out.GetStreamPosition();
			assetReader.SetStreamPosition(0);
			std::vector<char> buffer(size);
			assetReader.ReadData(buffer.data(), size);
			out.WriteData(buffer.data(), size);
			finalEntries[handle] = { offset, size };
		}

		out.SetStreamPosition(startIndex);
		for (auto& [handle, mtd] : _assetRegistry) {
			out.WriteData(reinterpret_cast<const char*>(&handle), sizeof(AssetHandle));
			out.WriteData(reinterpret_cast<const char*>(&finalEntries[handle].offset), sizeof(uint64_t));
			out.WriteData(reinterpret_cast<const char*>(&finalEntries[handle].size), sizeof(uint64_t));
			out.WriteData(reinterpret_cast<const char*>(&mtd.type), sizeof(AssetType));
		}
		GE_CORE_INFO("PAK File created at {}", outPath.string());
	}

	void EditorAssetManager::CompileIntoManifest(const std::filesystem::path& outPath)
	{
		GE_PROFILE_SCOPE("EditorAssetManager::CompileIntoManifest");

		file::Writer out(outPath);
		size_t assetCount = _assetRegistry.size();
		out.WriteData(reinterpret_cast<const char*>(&assetCount), sizeof(size_t));
		for (const auto& [handle, mtd] : _assetRegistry) {
			std::string pathStr = GetPathWithoutExtension(mtd.path);
			uint32_t pathLength = static_cast<uint32_t>(pathStr.size());
			out.WriteData(reinterpret_cast<const char*>(&pathLength), sizeof(uint32_t));
			out.WriteData(pathStr.c_str(), pathLength);
			out.WriteData(reinterpret_cast<const char*>(&handle), sizeof(AssetHandle));
		}
	}

	void EditorAssetManager::CookAssets(const std::filesystem::path& outPath)
	{
		CompileIntoPakFile(outPath);
		std::filesystem::path manifestPath = outPath.parent_path() / (outPath.stem().string() + "_manifest");
		manifestPath.replace_extension(".bin");
		CompileIntoManifest(manifestPath);
	}

	bool EditorAssetManager::AssetInRegistry(AssetHandle handle) { return _assetRegistry.contains(handle); }
	bool EditorAssetManager::AssetLoaded(AssetHandle handle) { return _loadedAssets.contains(handle); }
	AssetMetadata EditorAssetManager::GetMetadata(AssetHandle handle)
	{
		if (_assetRegistry.contains(handle))
			return _assetRegistry[handle];
		return AssetMetadata();
	}

	AssetMetadata EditorAssetManager::GetMetadata(const std::filesystem::path& path)
	{
		for (auto [handle, mtd] : _assetRegistry)
			if (mtd.path == path)
				return mtd;
		return AssetMetadata();
	}

	void EditorAssetManager::SaveAssetRegistry()
	{
		file::Writer out(_assetRegistryPath);
		if (!out.IsStreamGood()) {
			GE_CORE_ERROR("Failed to open asset registry");
			return;
		}

		size_t registrySize = _assetRegistry.size();
		out.WriteData(reinterpret_cast<const char*>(&registrySize), sizeof(size_t));
		for (const auto& [handle, mtd] : _assetRegistry) {
			size_t pathLength = mtd.path.string().size();
			out.WriteData(reinterpret_cast<const char*>(&handle),     sizeof(AssetHandle));
			out.WriteData(reinterpret_cast<const char*>(&mtd.handle), sizeof(AssetHandle));
			out.WriteData(reinterpret_cast<const char*>(&mtd.type),   sizeof(uint8_t));
			out.WriteData(reinterpret_cast<const char*>(&pathLength), sizeof(size_t));
			out.WriteData(mtd.path.string().c_str(), pathLength);
		}
	}

	void EditorAssetManager::LoadAssetRegistry()
	{
		file::Reader in(_assetRegistryPath);
		if (!in.IsStreamGood()) {
			GE_CORE_ERROR("Failed to open asset registry");
			return;
		}
		_assetRegistry.clear();

		size_t registrySize = 0;
		in.ReadData(reinterpret_cast<char*>(&registrySize), sizeof(size_t));
		for (size_t i = 0; i < registrySize; i++) {
			AssetHandle handle;
			AssetMetadata mtd;
			size_t pathLength = 0;
			in.ReadData(reinterpret_cast<char*>(&handle),     sizeof(AssetHandle));
			in.ReadData(reinterpret_cast<char*>(&mtd.handle), sizeof(AssetHandle));
			in.ReadData(reinterpret_cast<char*>(&mtd.type),	  sizeof(uint8_t));
			in.ReadData(reinterpret_cast<char*>(&pathLength), sizeof(size_t));
			std::string pathStr(pathLength, '\0');
			in.ReadData(pathStr.data(), pathLength);
			mtd.path = pathStr;

			_assetRegistry[handle] = mtd;
		}
		GE_CORE_INFO("Loaded {} assets from asset registry", registrySize);
	}
}