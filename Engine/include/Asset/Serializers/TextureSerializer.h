#pragma once
#include "AssetSerializer.h"

namespace ge {
#if 0
	class SourceTextureSerializer : public SourceSerializer {
	public:
		virtual AssetType ImportFromSource(const ImportAssetData& asset, const filesystem::Path& source, const filesystem::Path& targetPath) override;
		static mem::Ref<Asset> ImportTextureFromFile(const ImportAssetData& asset, const filesystem::Path& source);
	};

	class TextureSerializer : public AssetSerializer {
	public:
		virtual void SerializeToFile(const filesystem::Path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) override {};
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const GEVector<uint8_t>& buffer) override;
	};
#endif
}