#include "gepch.h"
#include "Material.h"

namespace ge::renderer {
	void MaterialAsset::SetAlbedoTexture(const AssetHandle& textureHandle) {
		_textureHandles.albedoTextureHandle = textureHandle;
		auto texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		if (texture)
			_material->_bindlessData.albedoTextureIndex = texture->GetHandle();
	}

	void MaterialAsset::SetRoughnessTexture(const AssetHandle& textureHandle) {
		_textureHandles.roughnessTextureHandle = textureHandle;
		auto texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		if (texture)
			_material->_bindlessData.roughnessTextureIndex = texture->GetHandle();
	}
	
	void MaterialAsset::SetNormalTexture(const AssetHandle& textureHandle) {
		_textureHandles.normalTextureHandle = textureHandle;
		auto texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		if (texture)
			_material->_bindlessData.normalTextureIndex = texture->GetHandle();
	}

	ge::mem::Ref<Material>& MaterialAsset::GetMaterial()
	{
		if (_textureHandles.albedoTextureHandle) {
			auto texture = AssetManager::GetAsset<Texture2D>(_textureHandles.albedoTextureHandle);
			if (texture) {
				_material->_bindlessData.albedoTextureIndex = texture->GetHandle();
			}
		}

		if (_textureHandles.roughnessTextureHandle) {
			auto texture = AssetManager::GetAsset<Texture2D>(_textureHandles.roughnessTextureHandle);
			if (texture) {
				_material->_bindlessData.roughnessTextureIndex = texture->GetHandle();
			}
		}

		if (_textureHandles.normalTextureHandle) {
			auto texture = AssetManager::GetAsset<Texture2D>(_textureHandles.normalTextureHandle);
			if (texture) {
				_material->_bindlessData.normalTextureIndex = texture->GetHandle();
			}
		}

		return _material;
	}
}