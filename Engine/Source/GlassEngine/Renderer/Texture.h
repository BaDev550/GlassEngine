#pragma once
#include "GlassEngine/Renderer/Image.h"
#include "GlassEngine/Renderer/RenderObject.h"
#include <filesystem>

namespace ge::renderer {
	struct TextureSpecification {
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
		mem::Ref<Image> _image = nullptr;
		TextureSpecification _specs;
	};

	class Texture2D : public Texture {
	public:
		virtual ~Texture2D() = default;
		static ge::mem::Ref<Texture2D> Create(const TextureSpecification& spec);
		static ge::mem::Ref<Texture2D> Create(const TextureSpecification& spec, const std::filesystem::path& filePath);
	};
}