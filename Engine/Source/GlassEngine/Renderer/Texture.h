#pragma once
#include "GlassEngine/Renderer/Image.h"
#include "GlassEngine/Renderer/RenderObject.h"
#include <filesystem>

#include "GlassEngine/Asset/Asset.h"

namespace ge::renderer {
	// TODO(0x): this just for Texture2D
	struct TextureSpec {
		uint32_t width = 1;
		uint32_t height = 1;
		ImageFilter filter = ImageFilter::Linear;
		ImageFormat format = ImageFormat::RGBA8;
		bool attachment = false;
		bool flipV = false;
		bool compress = false;
	};

	class Texture : public RenderObject {
	public:
		Texture(ge::mem::Ref<Image> image, ImageSubresource subresource, const TextureSpec& specs) : _image(image), _subresource(subresource), _specs(specs) { }
		virtual ~Texture() = default;
		ImageFormat GetImageFormat() const { return _specs.format; }
		ImageFilter GetImageFilter() const { return _specs.filter; }
		uint32_t GetWidth() const { return _specs.width; }
		uint32_t GetHeight() const { return _specs.height; }
		ge::mem::Ref<Image>& GetImage() { return _image; }
		const ge::mem::Ref<Image>& GetImage() const noexcept { return _image; }
		auto GetSubresource() const noexcept { return _subresource; }
		[[nodiscard]] uint32_t GetHandle();

		virtual void SetDebugName(GEString name) const noexcept override final {
			_image->SetDebugName(name);
		}
	protected:
		ge::mem::Ref<Image> _image = nullptr;
		ImageSubresource _subresource{};
		TextureSpec _specs;
		uint32_t _handle{ static_cast<uint32_t>(-1) };
	};

	class Texture2D : public Texture, public Asset {
	public:
		Texture2D(ge::mem::Ref<Image> image, ImageSubresource subresource, const TextureSpec& specs) : Texture(image, subresource, specs) {}
		virtual ~Texture2D() = default;
		static ge::mem::Ref<Texture2D> Create(const TextureSpec& spec);
		static ge::mem::Ref<Texture2D> Create(const TextureSpec& spec, const std::filesystem::path& filePath);
		static ge::mem::Ref<Texture2D> Create(const TextureSpec& spec, const void* data);
		static ge::mem::Ref<Texture2D> Create(ge::mem::Ref<Image> image, uint16_t layerIndex, uint16_t mipmapIndex);
		
		[[nodiscard]] auto& GetData() const noexcept { return _data; }
		[[nodiscard]] auto& GetData() noexcept { return _data; }

		static AssetType GetStaticAssetType() { return AssetType::Texture; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
	private:
		std::vector<uint8_t> _data;
	};
}
