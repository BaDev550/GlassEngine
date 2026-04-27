#include "gepch.h"
#include "TextureSerializer.h"
#include "GlassEngine/Renderer/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "compressonator.h"

namespace comp {
    static bool CompressionCallback(float fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2) {
        std::printf("\rCompression progress = %3.0f  ", fProgress);
        return false;
    }

    static void CompressToBc7(uint8_t* image, uint16_t x, uint16_t y, std::vector<uint8_t>& out_image) {
        CMP_Texture src{};
        src.dwSize = sizeof(CMP_Texture);
        src.format = CMP_FORMAT_RGBA_8888;
        src.dwWidth = x;
        src.dwHeight = y;
        src.dwDataSize = x * y * 4;
        src.dwPitch = x * 4;
        src.pData = static_cast<CMP_BYTE*>(image);

        CMP_Texture dst{};
        dst.dwSize = sizeof(CMP_Texture);
        dst.format = CMP_FORMAT_BC7;
        dst.dwWidth = x;
        dst.dwHeight = y;
        dst.dwPitch = 0;
        dst.dwDataSize = CMP_CalculateBufferSize(&dst);
        out_image.resize(dst.dwDataSize);
        dst.pData = static_cast<CMP_BYTE*>(out_image.data());

        CMP_CompressOptions options;
        options.dwSize = sizeof(options);
        options.fquality = 0.0;
        options.dwnumThreads = 10;

        CMP_ConvertTexture(&src, &dst, &options, CompressionCallback);
    }
}

namespace ge {
    // Source texture
    AssetType SourceTextureSerializer::ImportFromSource(const ImportAssetData& asset, const std::filesystem::path& source, const std::filesystem::path& targetPath) { // TODO(badev): rewrite this
        int width, height, channels;
        stbi_set_flip_vertically_on_load(asset.textureSpecs->flipV);
        uint8_t* pixels = stbi_load(source.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
        std::vector<uint8_t> outData;
        comp::CompressToBc7(pixels, width, height, outData);
        stbi_image_free(pixels);

        if (!pixels) {
            GE_CORE_ERROR("Failed to load source image: {}", source.string());
            return GE_INVALID_ASSET_TYPE;
        }

        file::Writer out(targetPath);
        if (!out.IsStreamGood()) {
            outData.clear();
            return GE_INVALID_ASSET_TYPE;
        }
        size_t dataSize = outData.size();

        out.WriteData(TEXTURE_MAGIC, 4);
        out.WriteData(reinterpret_cast<const char*>(&width), sizeof(uint32_t));
        out.WriteData(reinterpret_cast<const char*>(&height), sizeof(uint32_t));
        out.WriteData(reinterpret_cast<const char*>(asset.textureSpecs), sizeof(renderer::TextureSpec));
        out.WriteData(reinterpret_cast<const char*>(&dataSize), sizeof(uint32_t));
        out.WriteData(reinterpret_cast<const char*>(outData.data()), outData.size());

        outData.clear();
        return AssetType::Texture;
    }

    // Texture serializer
    mem::Ref<Asset> TextureSerializer::DeserializeFromFile(const AssetMetadata& mtd) {
        GE_PROFILE_SCOPE(("TextureSerializer::DeserializeFromFile " + mtd.path.string()).c_str());

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

        uint32_t dataSize;
        in.ReadData(reinterpret_cast<char*>(&dataSize), sizeof(uint32_t));
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