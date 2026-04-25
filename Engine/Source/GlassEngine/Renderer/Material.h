#pragma once
#include "GlassEngine/Asset/Asset.h"
#include "RenderObject.h"

namespace ge::renderer {
	class Material : public RenderObject {
	public:
		// This needs to have bindless ids for textures
	private:
		struct MaterialBindlessData {
			uint32_t albedoTextureIndex = 0;
			uint32_t roughnessTextureIndex = 0;
			uint32_t normalTextureIndex = 0;
		} _bindlessData;
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
	};

	class MaterialTable : public ge::mem::RefCounted {
	public:
		void AddMaterial(const AssetHandle& materialHandle, const ge::mem::Ref<MaterialAsset>& material) {
			_materials[materialHandle] = material;
		}

		ge::mem::Ref<Material> GetMaterial(const AssetHandle& materialHandle) const {
			auto it = _materials.find(materialHandle);
			if (it != _materials.end()) {
				return it->second->GetMaterial();
			}
			return nullptr;
		}
	private:
		GEUnorderedMap<AssetHandle, ge::mem::Ref<MaterialAsset>> _materials;
	};
}