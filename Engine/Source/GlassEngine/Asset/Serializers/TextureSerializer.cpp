#include "gepch.h"
#include "TextureSerializer.h"
#include "GlassEngine/Renderer/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace ge {
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
        out.WriteData(reinterpret_cast<const char*>(&width), sizeof(uint32_t));
        out.WriteData(reinterpret_cast<const char*>(&height), sizeof(uint32_t));
        out.WriteData(reinterpret_cast<const char*>(&asset), sizeof(renderer::TextureSpec));
        out.WriteData(reinterpret_cast<const char*>(pixels), dataSize);

        stbi_image_free(pixels);
        return AssetType::Texture;
    }

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
        renderer::TextureSpec textureSpecs{};
        in.ReadData(reinterpret_cast<char*>(&width), sizeof(uint32_t));
        in.ReadData(reinterpret_cast<char*>(&height), sizeof(uint32_t));
        in.ReadData(reinterpret_cast<char*>(&textureSpecs), sizeof(renderer::TextureSpec));
        textureSpecs.width = width;
        textureSpecs.height = height;
        textureSpecs.format = renderer::ImageFormat::RGBA8;
        textureSpecs.filter = renderer::ImageFilter::Linear;

        size_t dataSize = width * height * STBI_rgb_alpha;
        GEVector<uint8_t> pixelData(dataSize);
        in.ReadData(reinterpret_cast<char*>(pixelData.data()), dataSize);

        mem::Ref<renderer::Texture2D> texture = renderer::Texture2D::Create(textureSpecs, pixelData.data());
        return texture;
    }

    mem::Ref<Asset> TextureSerializer::DeserializeFromFile(const GEVector<uint8_t>& buffer)
    {
        file::BufferReader in(buffer);
        char magic[4];
        if (!in.ReadData(magic, 4) || strncmp(magic, TEXTURE_MAGIC, 4) != 0) {
            GE_CORE_ERROR("Invalid magic for texture asset");
            return nullptr;
        }
        uint32_t width, height;
        if (!in.Read(width) || !in.Read(height)) {
            GE_CORE_ERROR("Failed to read dimensions from buffer");
            return nullptr;
        }
        renderer::TextureSpec specs;
        if (!in.Read(specs)) {
            GE_CORE_ERROR("Failed to read specs");
            return nullptr;
        }
        specs.width = width;
        specs.height = height;
        specs.format = renderer::ImageFormat::RGBA8;
		specs.filter = renderer::ImageFilter::Linear;

        size_t dataSize = width * height * STBI_rgb_alpha; // STBI_rgb_alpha TEMP
        const uint8_t* pixelData = in.ReadPtr(dataSize);
        if (!pixelData) {
            GE_CORE_ERROR("No texture data found in buffer or buffer size mismacth with image size");
            return nullptr;
        }
        mem::Ref<renderer::Texture2D> texture = renderer::Texture2D::Create(specs, pixelData);
        return texture;
    }
}