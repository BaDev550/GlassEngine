#include "gepch.h"
#include "AssetSerializer.h"
#include "AssetExtensions.h"
#include "GlassEngine/Serialization/FileSerializer.h"
#include "GlassEngine/Renderer/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace ge {
	// Source serialzers

	// Source texture
	AssetType SourceTextureSerializer::ImportFromSource(const ImportAssetData& asset, const std::filesystem::path& source, const std::filesystem::path& targetPath) {
        int width, height, channels;
        stbi_uc* pixels = stbi_load(source.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            GE_CORE_ERROR("Failed to load source image: {}", source.string());
            return GE_INVALID_ASSET_TYPE;
        }

        file::Writer out(targetPath);
        if (!out.IsStreamGood()) {
            stbi_image_free(pixels);
            return GE_INVALID_ASSET_TYPE;
        }
        size_t dataSize = width * height * STBI_rgb_alpha;

        out.WriteData(TEXTURE_MAGIC, 4);
        out.WriteData(reinterpret_cast<const char*>(&width),  sizeof(uint32_t));
        out.WriteData(reinterpret_cast<const char*>(&height), sizeof(uint32_t));
        out.WriteData(reinterpret_cast<const char*>(&asset),  sizeof(renderer::TextureSpecification));
        out.WriteData(reinterpret_cast<const char*>(pixels),  dataSize);

        stbi_image_free(pixels);
        return AssetType::Texture;
	}

    // Serializers

    // Texture serializer
    mem::Ref<Asset> TextureSerializer::DeserializeFromFile(const AssetMetadata& mtd) {
        file::Reader in(mtd.path);
        if (!in.IsStreamGood()) {
            GE_CORE_ERROR("TextureSerializer could not open file: {}", mtd.path.string());
            return nullptr;
        }

        char magic[4];
        in.ReadData(magic, 4);
        if (strncmp(magic, TEXTURE_MAGIC, 4) != 0) {
            GE_CORE_ERROR("Invalid magic bytes for texture asset!");
            return nullptr;
        }

        uint32_t width, height;
        renderer::TextureSpecification textureSpecs;
        in.ReadData(reinterpret_cast<char*>(&width),  sizeof(uint32_t));
        in.ReadData(reinterpret_cast<char*>(&height), sizeof(uint32_t));
        in.ReadData(reinterpret_cast<char*>(&textureSpecs),  sizeof(renderer::TextureSpecification));
        textureSpecs.width = width;
        textureSpecs.height = height;

        size_t dataSize = width * height * STBI_rgb_alpha;
        std::vector<uint8_t> pixelData(dataSize);
        in.ReadData(reinterpret_cast<char*>(pixelData.data()), dataSize);

        mem::Ref<renderer::Texture2D> texture = renderer::Texture2D::Create(*asset.textureSpecs, pixelData.data());
        return texture;
    }

    mem::Ref<Asset> TextureSerializer::DeserializeFromFile(const std::vector<uint8_t>& buffer)
    {
        mem::Ref<renderer::Texture2D> texture = renderer::Texture2D::Create(*asset.textureSpecs, buffer.data());
        return texture;
    }
}