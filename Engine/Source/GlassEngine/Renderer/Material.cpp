#include "gepch.h"
#include "Material.h"

namespace ge::renderer {
	void MaterialAsset::SetAlbedoTexture(const AssetHandle& textureHandle) {
		_textureHandles.albedoTextureHandle = textureHandle;
		auto texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		if (texture) {
			_material->_bindlessData.albedoTextureIndex = texture->GetHandle();
			AssetManager::Editor_RegisterDependency(textureHandle, _assetHandle);
		}
	}

	void MaterialAsset::SetRoughnessTexture(const AssetHandle& textureHandle) {
		_textureHandles.roughnessTextureHandle = textureHandle;
		auto texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		if (texture) {
			_material->_bindlessData.roughnessTextureIndex = texture->GetHandle();
			AssetManager::Editor_RegisterDependency(textureHandle, _assetHandle);
		}
	}
	
	void MaterialAsset::SetNormalTexture(const AssetHandle& textureHandle) {
		_textureHandles.normalTextureHandle = textureHandle;
		auto texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		if (texture) {
			_material->_bindlessData.normalTextureIndex = texture->GetHandle();
			AssetManager::Editor_RegisterDependency(textureHandle, _assetHandle);
		}
	}

	ge::mem::Ref<Material>& MaterialAsset::GetMaterial()
	{
		return _material;
	}

	void MaterialAsset::OnDependencyUpdated(AssetHandle handle)
	{
		if (handle == _textureHandles.albedoTextureHandle) {
			auto texture = AssetManager::GetAsset<Texture2D>(_textureHandles.albedoTextureHandle);
			_material->_bindlessData.albedoTextureIndex = texture->GetHandle();
		}
		if (handle == _textureHandles.roughnessTextureHandle) {
			auto texture = AssetManager::GetAsset<Texture2D>(_textureHandles.roughnessTextureHandle);
			_material->_bindlessData.roughnessTextureIndex = texture->GetHandle();
		}
		if (handle == _textureHandles.normalTextureHandle) {
			auto texture = AssetManager::GetAsset<Texture2D>(_textureHandles.normalTextureHandle);
			_material->_bindlessData.normalTextureIndex = texture->GetHandle();
		}
	}
}