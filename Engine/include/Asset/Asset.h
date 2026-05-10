#pragma once
#include <Memory/Memory.h>
#include <UUID.h>

namespace ge {
	enum class AssetType : uint8_t {
		Unknown = 0,
		SourceMesh,
		SourceTexture,
		SourceShader,
		CompiledAsset,
		Texture,
		StaticMesh,
		Material,
		Scene
		//SkeletalMesh, if u needed add this and Skeletal enum
	};
#define GE_INVALID_ASSET_HANDLE 0
#define GE_INVALID_ASSET_TYPE AssetType::Unknown

	constexpr std::string AssetTypeToString(AssetType type) noexcept {
		switch (type)
		{
		case AssetType::Unknown: return "Unknown";
		case AssetType::SourceTexture: return "SourceTexture";
		case AssetType::SourceShader: return "SourceShader";
		case AssetType::SourceMesh: return "SourceMesh";
		case AssetType::StaticMesh: return "StaticMesh";
		case AssetType::Texture: return "Texture";
		case AssetType::Material: return "Material";
		case AssetType::Scene: return "Scene";
		default:
			return "Unknown";
		}
	}

	constexpr AssetType StringToAssetType(std::string_view type) noexcept {
		if (type == "Unknown") return AssetType::Unknown;
		if (type == "SourceMesh") return AssetType::SourceMesh;
		if (type == "SourceTexture") return AssetType::SourceTexture;
		if (type == "SourceShader") return AssetType::SourceShader;
		if (type == "StaticMesh") return AssetType::StaticMesh;
		if (type == "Material") return AssetType::Material;
		if (type == "Texture") return AssetType::Texture;
		if (type == "Scene") return AssetType::Scene;
		return GE_INVALID_ASSET_TYPE;
	}

	using AssetHandle = UUID;
	class Asset : public virtual mem::RefCounted {
	public:
		virtual ~Asset() = default;
		AssetHandle _assetHandle = GE_INVALID_ASSET_HANDLE;
		static AssetType GetStaticAssetType() { return GE_INVALID_ASSET_TYPE; }
		virtual AssetType GetAssetType() const { return GetStaticAssetType(); }
		virtual void OnDependencyUpdated(AssetHandle handle) {}
	};

	struct GAssetHeader {
		char magic[4];
		AssetType type;
	};

	struct PackedAsset {
		AssetHandle handle;
		uint64_t offset;
		uint64_t size;
		AssetType type;
	};

	namespace renderer {
		struct TextureSpec;
		struct SourceMeshSpec;
	}

	struct ImportAssetData {
		renderer::TextureSpec* textureSpecs;
		renderer::SourceMeshSpec* sourceMeshSpecs;
	};
}