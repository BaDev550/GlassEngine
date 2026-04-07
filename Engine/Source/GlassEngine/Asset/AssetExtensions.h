#pragma once
#include "Asset.h"
#include <map>

namespace ge {
#define GE_ASSET_EXTENSION ".gasset"
	constexpr char MODEL_MAGIC[4] = { 'M', 'O', 'D', 'L' };
	constexpr char TEXTURE_MAGIC[4] = { 'T', 'E', 'X', 'T' };
	constexpr char MATERIAL_MAGIC[4] = { 'M', 'A', 'T', 'L' };

	static std::map<std::string, AssetType> s_extensionAssetMap = {
		{ ".scene", AssetType::Scene },

		{ ".png",  AssetType::SourceTexture },
		{ ".jpg",  AssetType::SourceTexture },
		{ ".jpeg", AssetType::SourceTexture },
		{ ".tga",  AssetType::SourceTexture },
		{ ".bmp",  AssetType::SourceTexture },

		{ ".fbx",  AssetType::SourceMesh },
		{ ".obj",  AssetType::SourceMesh },
		{ ".gltf", AssetType::SourceMesh },
		{ ".glb",  AssetType::SourceMesh },
		{ ".dae",  AssetType::SourceMesh },

		{ ".gasset",  AssetType::CompiledAsset }
	};

	static std::map<std::string, AssetType> s_magicAssetMap = {
		{ MODEL_MAGIC, AssetType::StaticMesh },
		{ TEXTURE_MAGIC, AssetType::Texture },
		{ MATERIAL_MAGIC, AssetType::Material },
	};
}