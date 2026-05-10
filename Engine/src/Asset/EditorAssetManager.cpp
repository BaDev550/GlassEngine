#include <Application.h>

#include "Asset/AssetManager.h"
#include "Asset/AssetExtensions.h"
#include "Project/Project.h"

namespace ge {
	EditorAssetManager::EditorAssetManager(const filesystem::Path& assetRegistry) : _assetRegistryPath(assetRegistry)
	{
		if (!LoadAssetRegistry())
			TryToRebuildRegistry();
	}

	mem::Ref<Asset> EditorAssetManager::GetOrImportAsset(filesystem::Path sourcePath, ImportAssetData asset, filesystem::Path targetPath, AssetHandle handle) {
		if (handle != GE_INVALID_ASSET_HANDLE) {
			if (AssetLoaded(handle))
				return _loadedAssets.at(handle);
			else
				return LoadAssetFromFile(handle);
		}
		else if (!sourcePath.empty()) {
			auto assetPath = TryToGetGassetFile(sourcePath);
			auto mtd = GetMetadata(assetPath);
			if (mtd.IsValid()) {
				if (mtd.IsLoaded())
					return _loadedAssets.at(mtd.handle);
				else
					return LoadAssetFromFile(mtd.handle);
			}
			else {
				AssetHandle newHandle = ImportAsset(asset, assetPath, targetPath);
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

	AssetHandle EditorAssetManager::ImportAsset(const ImportAssetData& asset, filesystem::Path sourcePath, filesystem::Path targetPath)
	{
		if (!filesystem::FileSystem::Exists(sourcePath)) {
			GE_CORE_ERROR("Cannot import asset. Source file does not exist: {}", sourcePath.string());
			return GE_INVALID_ASSET_HANDLE;
		}

		if (targetPath.empty())
			targetPath = sourcePath;

		AssetType compiledType = _importer->ImportToGAsset(asset, sourcePath, targetPath);
		AssetHandle handle = AssetHandle(targetPath.string());
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

	AssetHandle EditorAssetManager::CreateAsset(const filesystem::Path& targetPath, mem::Ref<Asset> asset)
	{
		AssetMetadata mtd;
		mtd.handle = asset->_assetHandle;
		mtd.path = targetPath;
		mtd.type = asset->GetAssetType();
		mtd.state = AssetLoadingState::Loaded;

		_importer->SerializeToFile(targetPath, asset, mtd);

		_assetRegistry[asset->_assetHandle] = mtd;
		_loadedAssets[asset->_assetHandle] = asset;
		SaveAssetRegistry();

		GE_CORE_INFO("Created new asset: {} at {}", AssetTypeToString(mtd.type), targetPath.string());
		return asset->_assetHandle;
	}

	void EditorAssetManager::RegisterDependency(AssetHandle handle, AssetHandle dependency) {
		_assetDependencies[handle].insert(dependency);
	}

	void EditorAssetManager::UpdateDependencies(AssetHandle handle) {
		std::unordered_set<AssetHandle> dependencies;
		{
			if (auto it = _assetDependencies.find(handle); it != _assetDependencies.end())
				dependencies = it->second;
		}
		for (AssetHandle dependency : dependencies) {
			mem::Ref<Asset> asset = GetAsset(dependency);
			if (asset)
				asset->OnDependencyUpdated(handle);
		}
	}

	void EditorAssetManager::OnAssetDeleted(AssetHandle handle) {
		RemoveAsset(handle);
		SaveAssetRegistry();
	}

	AssetMap EditorAssetManager::GetLoadedAssetsWithType(AssetType type)
	{
		AssetMap result;
		for (auto& [id, asset] : _loadedAssets) {
			AssetMetadata mtd = GetMetadata(id);
			if (mtd.type == type)
				result[id] = asset;
		}
		return result;
	}

	void EditorAssetManager::RemoveAsset(AssetHandle handle)
	{
		if (_loadedAssets.contains(handle))
			_loadedAssets.erase(handle);
		if (_assetRegistry.contains(handle))
			_assetRegistry.erase(handle);
	}

	void EditorAssetManager::CompileIntoPakFile(const filesystem::Path& outPath)
	{
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
		GEMap<AssetHandle, TempEntry> finalEntries;

		for (auto& [handle, mtd] : _assetRegistry) { // Open all the files in registry and write into .pak
			file::Reader assetReader(mtd.path, std::ios::binary | std::ios::ate);
			if (!assetReader.IsStreamGood()) {
				GE_CORE_WARN("Failed to write asset to asset registry: {}", mtd.path.string());
				continue;
			}
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

	void EditorAssetManager::CompileIntoManifest(const filesystem::Path& outPath)
	{
		file::Writer out(outPath);
		size_t assetCount = _assetRegistry.size();
		out.WriteData(reinterpret_cast<const char*>(&assetCount), sizeof(size_t));
		for (const auto& [handle, mtd] : _assetRegistry) {
			GEString pathStr = GetPathWithoutExtension(mtd.path);
			uint32_t pathLength = static_cast<uint32_t>(pathStr.size());
			out.WriteData(reinterpret_cast<const char*>(&pathLength), sizeof(uint32_t));
			out.WriteData(pathStr.c_str(), pathLength);
			out.WriteData(reinterpret_cast<const char*>(&handle), sizeof(AssetHandle));
		}
	}

	void EditorAssetManager::CookAssets(const filesystem::Path& outPath)
	{
		CompileIntoPakFile(outPath);
		filesystem::Path manifestPath = outPath.GetParentPath() / (outPath.GetFileName() + "_manifest");
		manifestPath.ReplaceExtension(".bin");
		CompileIntoManifest(manifestPath);
	}

	void EditorAssetManager::DeleteAllGAssetFiles()
	{
		filesystem::FileSystem::IterateDirectory(Project::GetAssetDirectory(), [this](const filesystem::Path& path) {
			if (path.GetExtension() == GE_ASSET_EXTENSION) {
				AssetMetadata mtd = GetMetadata(path);
				RemoveAsset(mtd.handle);
			}
			});
		SaveAssetRegistry();
	}

	bool EditorAssetManager::AssetInRegistry(AssetHandle handle) { return _assetRegistry.contains(handle); }
	bool EditorAssetManager::AssetLoaded(AssetHandle handle) { return _loadedAssets.contains(handle); }
	AssetMetadata EditorAssetManager::GetMetadata(AssetHandle handle)
	{
		if (_assetRegistry.contains(handle))
			return _assetRegistry[handle];
		return AssetMetadata();
	}

	AssetMetadata EditorAssetManager::GetMetadata(const filesystem::Path& path)
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

	bool EditorAssetManager::LoadAssetRegistry()
	{
		file::Reader in(_assetRegistryPath);
		if (!in.IsStreamGood()) {
			GE_CORE_ERROR("Failed to open asset registry");
			return false;
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
			GEString pathStr(pathLength, '\0');
			in.ReadData(pathStr.data(), pathLength);
			mtd.path = pathStr;

			_assetRegistry[handle] = mtd;
		}
		GE_CORE_INFO("Loaded {} assets from asset registry", registrySize);
		return true;
	}

	bool EditorAssetManager::TryToRebuildRegistry()
	{
		GE_CORE_WARN("Trying to rebuild registry from asset dir");
		filesystem::FileSystem::IterateDirectory(Project::GetAssetDirectory(), [this](const filesystem::Path& path) {
			if (path.GetExtension() == GE_ASSET_EXTENSION) {
				file::Reader reader(path.c_str());

				GAssetHeader header{};
				reader.ReadData(reinterpret_cast<char*>(&header), sizeof(GAssetHeader));

				AssetHandle handle = AssetHandle(path.string());
				AssetMetadata mtd;
				mtd.handle = handle;
				mtd.path = path;
				mtd.type = header.type;

				_assetRegistry[handle] = mtd;
			}
			});
		SaveAssetRegistry();
		return true;
	}
}