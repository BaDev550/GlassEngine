#include "gepch.h"
#include "TextureSerializer.h"
#include "GlassEngine/Renderer/Texture.h"
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "compressonator.h"

namespace comp {
    static bool CompressionCallback(float fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2) {
        std::printf("\rCompression progress = %3.0f  ", fProgress);
        return false;
    }

    static void CompressToBc7(uint8_t* image, uint16_t x, uint16_t y, GEVector<uint8_t>& out_image, uint8_t& mipmapCount) {
        static bool cmpIsInitialized = false;
        if (!cmpIsInitialized) {
            CMP_InitFramework();
            cmpIsInitialized = true;
        }

        CMP_MipSet srcMipSet = {};
        CMP_ERROR err = CMP_CreateMipSet(&srcMipSet, x, y, 1, CF_8bit, TT_2D);
        if (err != CMP_OK) return;

        CMP_MipLevel* level0 = nullptr;
        CMP_GetMipLevel(&level0, &srcMipSet, 0, 0);

        if (!level0 || !level0->m_pbData) {
            CMP_FreeMipSet(&srcMipSet);
            return;
        }

        memcpy(level0->m_pbData, image, x * y * 4);

        CMP_GenerateMIPLevels(&srcMipSet, 0);
        if (err != CMP_OK) {
            CMP_FreeMipSet(&srcMipSet);
            return;
        }

        CMP_MipSet dstMipSet = {};

        KernelOptions kernelOptions = {};
        kernelOptions.format   = CMP_FORMAT_BC3;
        kernelOptions.fquality = 1.0f;
        kernelOptions.threads  = 0;

        err = CMP_ProcessTexture(&srcMipSet, &dstMipSet, kernelOptions, nullptr);
        if (err != CMP_OK) {
            CMP_FreeMipSet(&srcMipSet);
            return;
        }

        for (int i = 0; i < dstMipSet.m_nMipLevels; i++) {
            CMP_MipLevel* level = nullptr;
            CMP_GetMipLevel(&level, &dstMipSet, i, 0);
            if (!level || !level->m_pbData) continue;

            uint8_t* data = level->m_pbData;
            size_t   size = level->m_dwLinearSize;

            out_image.insert(out_image.end(), data, data + size);
        }

        mipmapCount = dstMipSet.m_nMipLevels - 2;

        CMP_FreeMipSet(&srcMipSet);
        CMP_FreeMipSet(&dstMipSet);
    }

    // static void CompressToBc7(uint8_t* image, uint16_t x, uint16_t y, GEVector<uint8_t>& out_image) {
    //     CMP_Texture src{};
    //     src.dwSize = sizeof(CMP_Texture);
    //     src.format = CMP_FORMAT_RGBA_8888;
    //     src.dwWidth = x;
    //     src.dwHeight = y;
    //     src.dwDataSize = x * y * 4;
    //     src.dwPitch = x * 4;
    //     src.pData = static_cast<CMP_BYTE*>(image);

    //     CMP_Texture dst{};
    //     dst.dwSize = sizeof(CMP_Texture);
    //     dst.format = CMP_FORMAT_BC3;
    //     dst.dwWidth = x;
    //     dst.dwHeight = y;
    //     dst.dwPitch = 0;
    //     dst.dwDataSize = CMP_CalculateBufferSize(&dst);
    //     out_image.resize(dst.dwDataSize);
    //     dst.pData = static_cast<CMP_BYTE*>(out_image.data());

    //     CMP_CompressOptions options{};
    //     options.dwSize = sizeof(options);
    //     options.fquality = 0.0;
    //     options.dwnumThreads = 8;

    //     CMP_ConvertTexture(&src, &dst, &options, CompressionCallback);
    // }
}

namespace ge {
    // Source texture
    AssetType SourceTextureSerializer::ImportFromSource(const ImportAssetData& asset, const std::filesystem::path& source, const std::filesystem::path& targetPath) { // TODO(badev): rewrite this
        int width, height, channels;
        bool compress = asset.textureSpecs->compress;
        stbi_set_flip_vertically_on_load(asset.textureSpecs->flipV);
        
        uint8_t* pixels = stbi_load(source.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!pixels) {
            GE_CORE_ERROR("Failed to load source image: {}", source.string());
            stbi_image_free(pixels);
            return GE_INVALID_ASSET_TYPE;
        }

        GEVector<uint8_t> outData;
        uint8_t mipmapCount = 1;
        if (asset.textureSpecs->compress) {
            comp::CompressToBc7(pixels, width, height, outData, mipmapCount);
        }
        else {
            outData.resize(width * height * STBI_rgb_alpha);
            std::memcpy(outData.data(), pixels, width * height * STBI_rgb_alpha);
        }

        file::Writer out(targetPath);
        if (!out.IsStreamGood()) {
            outData.clear();
            return GE_INVALID_ASSET_TYPE;
        }
        size_t dataSize = outData.size();
        asset.textureSpecs->width = width;
        asset.textureSpecs->height = height;
        asset.textureSpecs->mipmapCount = mipmapCount;

        GAssetHeader header{};
        std::memcpy(header.magic, TEXTURE_MAGIC, 4);
        header.type = AssetType::Texture;

        out.WriteData(reinterpret_cast<const char*>(&header), sizeof(GAssetHeader));
        out.WriteData(reinterpret_cast<const char*>(asset.textureSpecs), sizeof(renderer::TextureSpec));
        out.WriteData(reinterpret_cast<const char*>(&dataSize), sizeof(uint32_t));
        out.WriteData(reinterpret_cast<const char*>(outData.data()), outData.size());

        stbi_image_free(pixels);
        return AssetType::Texture;
    }

    mem::Ref<Asset> SourceTextureSerializer::ImportTextureFromFile(const ImportAssetData& asset, const std::filesystem::path& source)
    {
        int width, height, channels;
        renderer::TextureSpec specs{};
        if (asset.textureSpecs)
            specs = *asset.textureSpecs;
        stbi_set_flip_vertically_on_load(specs.flipV);

        uint8_t* pixels = stbi_load(source.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!pixels) {
            GE_CORE_ERROR("Failed to load source image: {}", source.string());
            return nullptr;
        }
        specs.width = width;
        specs.height = height;
        mem::Ref<renderer::Texture2D> texture = renderer::Texture2D::Create(specs, pixels);
        return texture;
    }

    // Texture serializer
    mem::Ref<Asset> TextureSerializer::DeserializeFromFile(const AssetMetadata& mtd) {
        GE_PROFILE_SCOPE(("TextureSerializer::DeserializeFromFile " + mtd.path.string()).c_str());

        file::Reader in(mtd.path);
        if (!in.IsStreamGood()) {
            GE_CORE_ERROR("TextureSerializer could not open file: {}", mtd.path.string());
            return nullptr;
        }

        GAssetHeader header;
        in.ReadData(reinterpret_cast<char*>(&header), sizeof(GAssetHeader));
        if (strncmp(header.magic, TEXTURE_MAGIC, 4) != 0) {
            GE_CORE_ERROR("Invalid magic bytes for texture asset!");
            return nullptr;
        }

        renderer::TextureSpec textureSpecs{};
        in.ReadData(reinterpret_cast<char*>(&textureSpecs), sizeof(renderer::TextureSpec));

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

        GAssetHeader header;
        if (!in.ReadData(reinterpret_cast<char*>(&header), sizeof(GAssetHeader)) || strncmp(header.magic, TEXTURE_MAGIC, 4) != 0) {
            GE_CORE_ERROR("Invalid magic for texture asset");
            return nullptr;
        }
        renderer::TextureSpec specs;
        if (!in.Read(specs)) {
            GE_CORE_ERROR("Failed to read specs");
            return nullptr;
        }
        uint32_t dataSize = 0;
        if (!in.Read(dataSize)) {
            GE_CORE_ERROR("Failed to read texture data size");
            return nullptr;
        }

        const uint8_t* pixelData = in.ReadPtr(dataSize);
        if (!pixelData) {
            GE_CORE_ERROR("No texture data found in buffer or buffer size mismacth with image size");
            return nullptr;
        }
        mem::Ref<renderer::Texture2D> texture = renderer::Texture2D::Create(specs, pixelData);
        return texture;
    }
}