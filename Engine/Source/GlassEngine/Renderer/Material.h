#pragma once
#include "GlassEngine/Asset/Asset.h"
#include "RenderObject.h"

namespace ge::renderer {
	struct MaterialBindlessData {
		uint32_t albedoTextureIndex = 0;
		uint32_t roughnessTextureIndex = 0;
		uint32_t normalTextureIndex = 0;
		uint32_t emissiveTextureIndex = 0;
	};

	struct MaterialData {
		glm::u8vec4 albedoColor = { 255, 255, 255, 255 };
		glm::u8vec4 emissiveColor = { 255, 255, 255, 255 };
		float roughness{ 0.4f };
		float metallic{ 0.5f };
		float emissiveIntensity{ 1.0f };
	};

	class Material : public RenderObject {
	public:
		MaterialBindlessData& GetBindlessData() { return _bindlessData; }
		// This needs to have bindless ids for textures

		virtual void SetDebugName(GEString name) const noexcept final override { };
	private:
		MaterialBindlessData _bindlessData;
		friend class MaterialAsset;
	};

	class MaterialAsset : public Asset {
	public:
		MaterialAsset(const ge::mem::Ref<Material>& material) : _material(material) { }
		void SetAlbedoTexture(const AssetHandle& textureHandle);
		void SetRoughnessTexture(const AssetHandle& textureHandle);
		void SetNormalTexture(const AssetHandle& textureHandle);

		void SetAlbedoColor(glm::u8vec4 color) { _materialData.albedoColor = color; }
		void SetEmissiveColor(glm::u8vec4 color) { _materialData.emissiveColor = color; }
		void SetRoughness(float roughness) { _materialData.roughness = roughness; }
		void SetMetallic(float metallic) { _materialData.metallic = metallic; }
		void SetEmissiveIntensity(float intensity) { _materialData.emissiveIntensity = intensity; }

		ge::mem::Ref<Material>& GetMaterial();
		static AssetType GetStaticAssetType() { return AssetType::Material; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
		virtual void OnDependencyUpdated(AssetHandle handle) override;
	private:
		struct TextureHandles {
			AssetHandle albedoTextureHandle = GE_INVALID_ASSET_HANDLE;
			AssetHandle roughnessTextureHandle = GE_INVALID_ASSET_HANDLE;
			AssetHandle normalTextureHandle = GE_INVALID_ASSET_HANDLE;
		} _textureHandles;
		MaterialData _materialData;
		ge::mem::Ref<Material> _material;
	public:
		TextureHandles& GetTextureHandles() { return _textureHandles; }
		MaterialData& GetMaterialData() { return _materialData; }
	};

	class MaterialTable : public ge::mem::RefCounted {
	public:
		void AddMaterial(uint32_t materialIndex, const ge::mem::Ref<MaterialAsset>& material) {
			_materials[materialIndex] = material;
		}

		GEUnorderedMap<uint32_t, ge::mem::Ref<MaterialAsset>>& GetMaterials() { return _materials; }
		ge::mem::Ref<Material> GetMaterial(uint32_t materialIndex) {
			auto it = _materials.find(materialIndex);
			if (it != _materials.end()) {
				return it->second->GetMaterial();
			}
			return nullptr;
		}
	private:
		GEUnorderedMap<uint32_t, ge::mem::Ref<MaterialAsset>> _materials;
	};
}