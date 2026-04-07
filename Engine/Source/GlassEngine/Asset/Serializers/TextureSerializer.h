#pragma once
#include "AssetSerializer.h"

namespace ge {
	class SourceTextureSerializer : public SourceSerializer {
	public:
		virtual AssetType ImportFromSource(const ImportAssetData& asset, const std::filesystem::path& source, const std::filesystem::path& targetPath) override;
	};

	class TextureSerializer : public AssetSerializer {
	public:
		virtual void SerializeToFile(const std::filesystem::path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) override {};
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const GEVector<uint8_t>& buffer) override;
	};
}