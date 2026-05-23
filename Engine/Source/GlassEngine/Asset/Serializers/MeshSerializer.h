#pragma once
#include "AssetSerializer.h"
#include <assimp/postprocess.h>

namespace ge {
	constexpr uint32_t BASE_ASSIMP_FLAGS = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
	
	class MeshAssetSerializer : public AssetSerializer {
	public:
		virtual ~MeshAssetSerializer() = default;
		virtual void SerializeToFile(const std::filesystem::path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const GEVector<uint8_t>& buffer) override;
	};

	class MeshSourceSerializer : public SourceSerializer {
	public:
		virtual AssetType ImportFromSource(const ImportAssetData& asset, const std::filesystem::path& source, const std::filesystem::path& targetPath) override;
	};
}