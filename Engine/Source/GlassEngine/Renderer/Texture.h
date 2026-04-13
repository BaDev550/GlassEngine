#pragma once
#include "GlassEngine/Renderer/Image.h"
#include "GlassEngine/Renderer/RenderObject.h"
#include <filesystem>

#include "GlassEngine/Asset/Asset.h"

namespace ge::renderer {
	struct TextureSpec {
		uint32_t width = 1;
		uint32_t height = 1;
		ImageFilter filter = ImageFilter::Linear;
		ImageFormat format = ImageFormat::RGBA8;
		bool attachment = false;
	};

	class Texture : public RenderObject {
	public:
		virtual ~Texture() = default;
		ImageFormat GetImageFormat() const { return _specs.format; }
		ImageFilter GetImageFilter() const { return _specs.filter; }
		uint32_t GetWidth() const { return _specs.width; }
		uint32_t GetHeight() const { return _specs.height; }
		mem::Ref<Image>& GetImage() { return _image; }
	protected:
		Texture(mem::Ref<Image> image, const TextureSpec& specs)
			: _image(image), _specs(specs) {}
		mem::Ref<Image> _image = nullptr;
		TextureSpec _specs;
	};

	class Texture2D : public Texture, public Asset {
	public:
		virtual ~Texture2D() = default;
		static ge::mem::Ref<Texture2D> Create(const TextureSpec& spec);
		static ge::mem::Ref<Texture2D> Create(const TextureSpec& spec, const std::filesystem::path& filePath);
		static ge::mem::Ref<Texture2D> Create(const TextureSpec& spec, const void* data);
		
		[[nodiscard]] auto& GetData() const noexcept { return _data; }
		[[nodiscard]] auto& GetData() noexcept { return _data; }
	private:
		Texture2D(mem::Ref<Image> image, const TextureSpec& specs) : Texture(image, specs) {}
		std::vector<uint8_t> _data;
	};
}
