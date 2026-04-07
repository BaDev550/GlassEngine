#pragma once
#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Utilities/UUID.h"

namespace ge {
	/* All the source mesh / textures are loaded from disk to engine
	* then selects the type for example for mesh:
	*	selected type is static mesh so it calls static mesh serializer to serialize new file
	*	into selected directory reseves first 4 byte for magic names
	*	for model its MODL then serializes the model as .gasset in disk
	*	when loading the mesh is selected from disk looks inside of the first 4 bytes to select witch is
	*	model or other type then processes the mesh or texture
	* 
	* constexpr char MODEL_MAGIC[4] = { 'M', 'O', 'D', 'L' };
	* constexpr char TEXTURE_MAGIC[4] = { 'T', 'E', 'X', 'T' };
	* constexpr char MATERIAL_MAGIC[4] = { 'M', 'A', 'T', 'L' };
	*/

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
	};
}