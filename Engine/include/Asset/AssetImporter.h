#pragma once
#include <Filesystem.h>

#include "Engine.h"
#include "Serializers/AssetSerializer.h"
#include "Serializers/TextureSerializer.h"
#include "Serializers/MeshSerializer.h"
#include "AssetExtensions.h"

namespace ge {
	class AssetImporter {
	public:
		AssetImporter() {
#if 0
			_sourceSerializers[AssetType::SourceTexture] = mem::Ref<SourceTextureSerializer>::Create();
			_sourceSerializers[AssetType::SourceMesh] = mem::Ref<MeshSourceSerializer>::Create();
			_serializers[AssetType::Texture] = mem::Ref<TextureSerializer>::Create();
			_serializers[AssetType::Material] = mem::Ref<MaterialSerializer>::Create();
			_serializers[AssetType::StaticMesh] = mem::Ref<MeshAssetSerializer>::Create();
#endif
		}
		AssetImporter(AssetImporter&) = delete;
		AssetImporter& operator=(AssetImporter&) = delete;

		void SerializeToFile(const filesystem::Path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) {
			if (_serializers.contains(mtd.type)) {
				_serializers[mtd.type]->SerializeToFile(target, asset, mtd);
			}
			else {
				GE_CORE_ERROR("Failed to find serializer for type: {}", AssetTypeToString(mtd.type));
			}
		}

		mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) {
			if (_serializers.contains(mtd.type)) {
				return _serializers[mtd.type]->DeserializeFromFile(mtd);
			}
			else {
				GE_CORE_ERROR("Failed to find serializer for type: {}", AssetTypeToString(mtd.type));
				return {};
			}
		}
		mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd, const GEVector<uint8_t>& buffer) {
			if (_serializers.contains(mtd.type)) {
				return _serializers[mtd.type]->DeserializeFromFile(buffer);
			}
			else {
				GE_CORE_ERROR("Failed to find serializer for type: {}", AssetTypeToString(mtd.type));
				return {};
			}
		}

		AssetType ImportToGAsset(const ImportAssetData& asset, const filesystem::Path& source, filesystem::Path& targetPath) {
			AssetType sourceType = GetAssetTypeFromExtension(source.GetExtension());
			if (_sourceSerializers.contains(sourceType)) {
				targetPath.ReplaceExtension(GE_ASSET_EXTENSION);
				return _sourceSerializers[sourceType]->ImportFromSource(asset, source, targetPath);
			}
			else {
				GE_CORE_ERROR("Failed to find serializer for type: {}", AssetTypeToString(sourceType));
				return {};
			}
		}
	private:
		GEUnorderedMap<AssetType, mem::Ref<AssetSerializer>> _serializers;
		GEUnorderedMap<AssetType, mem::Ref<SourceSerializer>> _sourceSerializers;
	};
}