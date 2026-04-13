#pragma once
#include "Asset.h"
#include <map>
#include <array>

namespace ge {
#define GE_ASSET_EXTENSION ".gasset"
    constexpr char MODEL_MAGIC[4] = { 'M', 'O', 'D', 'L' };
    constexpr char TEXTURE_MAGIC[4] = { 'T', 'E', 'X', 'T' };
    constexpr char MATERIAL_MAGIC[4] = { 'M', 'A', 'T', 'L' };

    constexpr std::array<std::pair<std::string_view, AssetType>, 12> s_extensionAssetMap = { {
            { ".scene",  AssetType::Scene },

            { ".png",    AssetType::SourceTexture },
            { ".jpg",    AssetType::SourceTexture },
            { ".jpeg",   AssetType::SourceTexture },
            { ".tga",    AssetType::SourceTexture },
            { ".bmp",    AssetType::SourceTexture },

            { ".fbx",    AssetType::SourceMesh },
            { ".obj",    AssetType::SourceMesh },
            { ".gltf",   AssetType::SourceMesh },
            { ".glb",    AssetType::SourceMesh },
            { ".dae",    AssetType::SourceMesh },

            { ".gasset", AssetType::CompiledAsset }
    } };

    inline AssetType GetAssetTypeFromExtension(std::string_view extension) {
        for (const auto& [ext, type] : s_extensionAssetMap) {
            if (ext == extension) {
                return type;
            }
        }
        return AssetType::Unknown;
    }
}