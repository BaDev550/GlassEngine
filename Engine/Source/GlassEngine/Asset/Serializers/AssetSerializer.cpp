#include "gepch.h"
#include "AssetSerializer.h"

namespace ge {
    void MaterialSerializer::SerializeToFile(const std::filesystem::path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd)
    {
		mem::Ref<renderer::MaterialAsset> materialAsset = asset.Cast<renderer::MaterialAsset>();

		file::Writer out(target);
		if (!out.IsStreamGood()) {
			GE_CORE_ERROR("Failed to write Material asset to: {}", target.string());
			return;
		}

		out.WriteData(MATERIAL_MAGIC, 4);

		auto& handles = materialAsset->GetTextureHandles();
		out.WriteData(reinterpret_cast<const char*>(&handles.albedoTextureHandle), sizeof(AssetHandle));
		out.WriteData(reinterpret_cast<const char*>(&handles.roughnessTextureHandle), sizeof(AssetHandle));
		out.WriteData(reinterpret_cast<const char*>(&handles.normalTextureHandle), sizeof(AssetHandle));
    }

    mem::Ref<Asset> MaterialSerializer::DeserializeFromFile(const AssetMetadata& mtd)
    {
		file::Reader in(mtd.path);
		if (!in.IsStreamGood()) return nullptr;

		char magic[4];
		in.ReadData(magic, 4);
		if (strncmp(magic, MATERIAL_MAGIC, 4) != 0) return nullptr;

		mem::Ref<renderer::Material> mat = mem::Ref<renderer::Material>::Create();
		mem::Ref<renderer::MaterialAsset> matAsset = mem::Ref<renderer::MaterialAsset>::Create(mat);

		AssetHandle albedo, roughness, normal;
		in.ReadData(reinterpret_cast<char*>(&albedo), sizeof(AssetHandle));
		in.ReadData(reinterpret_cast<char*>(&roughness), sizeof(AssetHandle));
		in.ReadData(reinterpret_cast<char*>(&normal), sizeof(AssetHandle));

		matAsset->SetAlbedoTexture(albedo);
		matAsset->SetRoughnessTexture(roughness);
		matAsset->SetNormalTexture(normal);

		return matAsset;
    }

    mem::Ref<Asset> MaterialSerializer::DeserializeFromFile(const GEVector<uint8_t>& buffer)
    {
        return mem::Ref<Asset>();
    }
}