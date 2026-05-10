#include "Asset/Serializers/AssetSerializer.h"

namespace ge {
#if 0
    void MaterialSerializer::SerializeToFile(const filesystem::Path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd)
    {
		mem::Ref<renderer::MaterialAsset> materialAsset = asset.Cast<renderer::MaterialAsset>();

		file::Writer out(target);
		if (!out.IsStreamGood()) {
			GE_CORE_ERROR("Failed to write Material asset to: {}", target.string());
			return;
		}
		GAssetHeader header{};
		std::memcpy(header.magic, MATERIAL_MAGIC, 4);
		header.type = AssetType::Material;
		out.WriteData(reinterpret_cast<const char*>(&header), sizeof(GAssetHeader));

		auto& handles = materialAsset->GetTextureHandles();
		auto& matData = materialAsset->GetMaterialData();
		out.WriteData(reinterpret_cast<const char*>(&handles.albedoTextureHandle), sizeof(AssetHandle));
		out.WriteData(reinterpret_cast<const char*>(&handles.roughnessTextureHandle), sizeof(AssetHandle));
		out.WriteData(reinterpret_cast<const char*>(&handles.normalTextureHandle), sizeof(AssetHandle));
		out.WriteData(reinterpret_cast<const char*>(&matData.albedoColor), sizeof(glm::u8vec4));
		out.WriteData(reinterpret_cast<const char*>(&matData.emissiveColor), sizeof(glm::u8vec4));
		out.WriteData(reinterpret_cast<const char*>(&matData.roughness), sizeof(float));
		out.WriteData(reinterpret_cast<const char*>(&matData.metallic), sizeof(float));
		out.WriteData(reinterpret_cast<const char*>(&matData.emissiveIntensity), sizeof(float));
    }																			  

    mem::Ref<Asset> MaterialSerializer::DeserializeFromFile(const AssetMetadata& mtd)
    {
		file::Reader in(mtd.path);
		if (!in.IsStreamGood()) return nullptr;

		GAssetHeader header{};
		in.ReadData(reinterpret_cast<char*>(&header), sizeof(GAssetHeader));
		if (strncmp(header.magic, MATERIAL_MAGIC, 4) != 0) return nullptr;

		mem::Ref<renderer::Material> mat = mem::Ref<renderer::Material>::Create();
		mem::Ref<renderer::MaterialAsset> matAsset = mem::Ref<renderer::MaterialAsset>::Create(mat);

		AssetHandle albedo, roughnessT, normal;
		glm::u8vec4 albedoColor, emissiveColor;
		float roughness, metallic, emissiveIntensity;
		in.ReadData(reinterpret_cast<char*>(&albedo), sizeof(AssetHandle));
		in.ReadData(reinterpret_cast<char*>(&roughnessT), sizeof(AssetHandle));
		in.ReadData(reinterpret_cast<char*>(&normal), sizeof(AssetHandle));
		in.ReadData(reinterpret_cast<char*>(&albedoColor), sizeof(glm::u8vec4));
		in.ReadData(reinterpret_cast<char*>(&emissiveColor), sizeof(glm::u8vec4));
		in.ReadData(reinterpret_cast<char*>(&roughness), sizeof(float));
		in.ReadData(reinterpret_cast<char*>(&metallic), sizeof(float));
		in.ReadData(reinterpret_cast<char*>(&emissiveIntensity), sizeof(float));

		matAsset->SetAlbedoTexture(albedo);
		matAsset->SetRoughnessTexture(roughnessT);
		matAsset->SetNormalTexture(normal);
		matAsset->SetAlbedoColor(albedoColor);
		matAsset->SetEmissiveColor(emissiveColor);
		matAsset->SetRoughness(roughness);
		matAsset->SetMetallic(metallic);
		matAsset->SetEmissiveIntensity(emissiveIntensity);

		return matAsset;
    }

    mem::Ref<Asset> MaterialSerializer::DeserializeFromFile(const GEVector<uint8_t>& buffer)
    {
		file::BufferReader in(buffer);
		if (!in.IsStreamGood()) return nullptr;

		GAssetHeader header{};
		in.ReadData(reinterpret_cast<char*>(&header), sizeof(GAssetHeader));
		if (strncmp(header.magic, MATERIAL_MAGIC, 4) != 0) return nullptr;

		mem::Ref<renderer::Material> mat = mem::Ref<renderer::Material>::Create();
		mem::Ref<renderer::MaterialAsset> matAsset = mem::Ref<renderer::MaterialAsset>::Create(mat);

		AssetHandle albedo, roughnessT, normal;
		glm::u8vec4 albedoColor, emissiveColor;
		float roughness, metallic, emissiveIntensity;
		in.ReadData(reinterpret_cast<char*>(&albedo), sizeof(AssetHandle));
		in.ReadData(reinterpret_cast<char*>(&roughnessT), sizeof(AssetHandle));
		in.ReadData(reinterpret_cast<char*>(&normal), sizeof(AssetHandle));
		in.ReadData(reinterpret_cast<char*>(&albedoColor), sizeof(glm::u8vec4));
		in.ReadData(reinterpret_cast<char*>(&emissiveColor), sizeof(glm::u8vec4));
		in.ReadData(reinterpret_cast<char*>(&roughness), sizeof(float));
		in.ReadData(reinterpret_cast<char*>(&metallic), sizeof(float));
		in.ReadData(reinterpret_cast<char*>(&emissiveIntensity), sizeof(float));

		matAsset->SetAlbedoTexture(albedo);
		matAsset->SetRoughnessTexture(roughnessT);
		matAsset->SetNormalTexture(normal);
		matAsset->SetAlbedoColor(albedoColor);
		matAsset->SetEmissiveColor(emissiveColor);
		matAsset->SetRoughness(roughness);
		matAsset->SetMetallic(metallic);
		matAsset->SetEmissiveIntensity(emissiveIntensity);

		return matAsset;
    }
#endif
}