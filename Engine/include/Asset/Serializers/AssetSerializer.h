#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetMetadata.h"
#include "Asset/AssetExtensions.h"
#include "Serialization/FileSerializer.h"
#include "Serialization/BufferReader.h"
#include <set>

namespace ge {
	class AssetSerializer : public mem::RefCounted {
	public:
		virtual ~AssetSerializer() = default;
		virtual void SerializeToFile(const filesystem::Path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) = 0;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) = 0;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const GEVector<uint8_t>& buffer) = 0;
	};

	class SourceSerializer : public mem::RefCounted {
	public:
		virtual AssetType ImportFromSource(const ImportAssetData& asset, const filesystem::Path& source, const filesystem::Path& targetPath) = 0;
	};

#if 0
	class MaterialSerializer : public AssetSerializer {
	public:
		virtual void SerializeToFile(const filesystem::Path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const GEVector<uint8_t>& buffer) override;
	};
#endif
}