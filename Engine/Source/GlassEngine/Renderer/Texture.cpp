#include "gepch.h"
#include "Texture.h"
#include "Renderer.h"

#include "stb_image.h"

namespace ge::renderer {
	namespace utility {
		size_t calculateMipmapTotalSize(int width, int height, int mipLevels, int bytesPerPixel) {
			size_t total = 0;
			for (int i = 0; i < mipLevels; i++) {
				int w = std::max(1, width  >> i);  // width  / 2^i
				int h = std::max(1, height >> i);  // height / 2^i
				total += (size_t)w * h * bytesPerPixel;
			}
			return total;
		}
	}

	uint32_t Texture::GetHandle() {
		if (_handle == static_cast<uint32_t>(-1)) {
			_handle = _renderContext.GetReadonlyImageHandle(*_image, _subresource);
		}
		return _handle;
	}

	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpec& spec)
	{
		ImageSpec createDesc{};
		createDesc.imageFormat = spec.format;
		createDesc.extent = { spec.width, spec.height, 1 };
		createDesc.usageFlags |= ImageUsageFlagsBits::Readonly;
		createDesc.usageFlags |= ImageUsageFlagsBits::TransferDst;
		if (spec.attachment)
			createDesc.usageFlags |= utility::IsColorFormat(spec.format) ? ImageUsageFlagsBits::ColorAttachment : ImageUsageFlagsBits::DepthStencilAttachment;

		return ge::mem::Ref<Texture2D>::Create(Image::Create(createDesc), ImageSubresource{}, spec);
	}

	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpec& spec, const std::filesystem::path& filePath)
	{
		return nullptr; // Textures are loaded from asset manager
	}

	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpec& spec, const void* data) {
		const auto* dataPtr = reinterpret_cast<const uint8_t*>(data);
		const auto dataSize = utility::calculateMipmapTotalSize(
			spec.width, 
			spec.height, 
			spec.mipmapCount,
			utility::GetPixelSize(spec.format));

		ImageSpec createDesc{};
		createDesc.imageFormat = spec.format;
		createDesc.extent = { spec.width, spec.height, 1 };
		createDesc.usageFlags |= ImageUsageFlagsBits::Readonly;
		createDesc.usageFlags |= ImageUsageFlagsBits::TransferDst;
		createDesc.mipmapCount = spec.mipmapCount;
		if (spec.attachment)
			createDesc.usageFlags |= utility::IsColorFormat(spec.format) ?
				ImageUsageFlagsBits::ColorAttachment : ImageUsageFlagsBits::DepthStencilAttachment;

		auto texture = ge::mem::Ref<Texture2D>::Create(
			Image::Create(createDesc), 
			ImageSubresource{.mipmapCount = spec.mipmapCount}, spec);
		texture->_data = std::vector<uint8_t>(dataPtr, dataPtr + dataSize);

		renderer::Renderer3D::Submit([texture]() mutable {
			Renderer3D::GetRenderAPI()->BeginCopyPass();
			Renderer3D::GetRenderAPI()->LoadDataToTexture2D(*texture, texture->_data.data(), texture->_data.size());
			Renderer3D::GetRenderAPI()->EndCopyPass();
			});
		return texture;
	}

	ge::mem::Ref<Texture2D> Texture2D::Create(ge::mem::Ref<Image> image, uint16_t layerIndex, uint16_t mipmapIndex) {
		const auto subresource = ImageSubresource{
			.baseLayer = layerIndex,
			.layerCount = 1,
			.baseMipmap = static_cast<uint8_t>(mipmapIndex),
			.mipmapCount = 1,
			.type = ImageSubresourceType::e2D
		};

		return ge::mem::Ref<Texture2D>::Create(image, subresource, TextureSpec());
	}
}
