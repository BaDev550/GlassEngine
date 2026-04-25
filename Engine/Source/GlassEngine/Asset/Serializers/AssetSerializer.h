#pragma once
#include "GlassEngine/Asset/Asset.h"
#include "GlassEngine/Asset/AssetMetadata.h"
#include "GlassEngine/Asset/AssetExtensions.h"
#include "GlassEngine/Serialization/FileSerializer.h"
#include "GlassEngine/Serialization/BufferReader.h"
#include <set>

namespace ge {
	class AssetSerializer : public mem::RefCounted {
	public:
		virtual ~AssetSerializer() = default;
		virtual void SerializeToFile(const std::filesystem::path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) = 0;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) = 0;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const GEVector<uint8_t>& buffer) = 0;
	};

	class SourceSerializer : public mem::RefCounted {
	public:
		virtual AssetType ImportFromSource(const ImportAssetData& asset, const std::filesystem::path& source, const std::filesystem::path& targetPath) = 0;
	};

	class MaterialSerializer : public AssetSerializer {
	public:
		virtual void SerializeToFile(const std::filesystem::path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const GEVector<uint8_t>& buffer) override;
	};
}