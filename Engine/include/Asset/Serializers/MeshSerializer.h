#pragma once
#include "AssetSerializer.h"
#include <assimp/postprocess.h>

namespace ge {
	constexpr uint32_t BASE_ASSIMP_FLAGS = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
	
#if 0
	class MeshAssetSerializer : public AssetSerializer {
	public:
		virtual ~MeshAssetSerializer() = default;
		virtual void SerializeToFile(const filesystem::Path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const AssetMetadata& mtd) override;
		[[nodiscard]] virtual mem::Ref<Asset> DeserializeFromFile(const GEVector<uint8_t>& buffer) override;
	};

	class MeshSourceSerializer : public SourceSerializer {
	public:
		virtual AssetType ImportFromSource(const ImportAssetData& asset, const filesystem::Path& source, const filesystem::Path& targetPath) override;
	};
#endif
}