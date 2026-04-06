#pragma once
#include "Asset.h"
#include <map>

namespace ge {
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
}