#pragma once
#include "AssetSerializer.h"
#include "AssetExtensions.h"

namespace ge {
	class AssetImporter {
	public:
		AssetImporter() {
			_sourceSerializers[AssetType::SourceTexture] = mem::Ref<SourceTextureSerializer>::Create();

			_serializers[AssetType::Texture] = mem::Ref<TextureSerializer>::Create();
		}
		AssetImporter(AssetImporter&) = delete;
		AssetImporter& operator=(AssetImporter&) = delete;

		void SerializeToFile(const std::filesystem::path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) {
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
			}
		}

		AssetType ImportToGAsset(const ImportAssetData& asset, const std::filesystem::path& source, std::filesystem::path& targetPath) {
			AssetType sourceType = s_extensionAssetMap[source.extension().string()];
			if (_sourceSerializers.contains(sourceType)) {
				targetPath.replace_extension(GE_ASSET_EXTENSION);
				return _sourceSerializers[sourceType]->ImportFromSource(asset, source, targetPath);
			}
			else {
				GE_CORE_ERROR("Failed to find serializer for type: {}", AssetTypeToString(sourceType));
			}
		}
	private:
		std::unordered_map<AssetType, mem::Ref<AssetSerializer>> _serializers;
		std::unordered_map<AssetType, mem::Ref<SourceSerializer>> _sourceSerializers;
	};
}