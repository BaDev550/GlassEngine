#pragma once
#include "GlassEngine/Asset/Asset.h"
#include "RenderObject.h"

namespace ge::renderer {
	struct MaterialBindlessData {
		uint32_t albedoTextureIndex = 0;
		uint32_t roughnessTextureIndex = 0;
		uint32_t normalTextureIndex = 0;
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
		MaterialAsset(const ge::mem::Ref<Material>& material) : _material(material) {}
		void SetAlbedoTexture(const AssetHandle& textureHandle);
		void SetRoughnessTexture(const AssetHandle& textureHandle);
		void SetNormalTexture(const AssetHandle& textureHandle);
		ge::mem::Ref<Material> GetMaterial() const { return _material; }
		static AssetType GetStaticAssetType() { return AssetType::Material; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
	private:
		struct TextureHandles {
			AssetHandle albedoTextureHandle = GE_INVALID_ASSET_HANDLE;
			AssetHandle roughnessTextureHandle = GE_INVALID_ASSET_HANDLE;
			AssetHandle normalTextureHandle = GE_INVALID_ASSET_HANDLE;
		} _textureHandles;
		ge::mem::Ref<Material> _material;
	public:
		TextureHandles& GetTextureHandles() { return _textureHandles; }
	};

	class MaterialTable : public ge::mem::RefCounted {
	public:
		void AddMaterial(uint32_t materialIndex, const ge::mem::Ref<MaterialAsset>& material) {
			_materials[materialIndex] = material;
		}

		GEUnorderedMap<uint32_t, ge::mem::Ref<MaterialAsset>>& GetMaterials() { return _materials; }
		ge::mem::Ref<Material> GetMaterial(uint32_t materialIndex) const {
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