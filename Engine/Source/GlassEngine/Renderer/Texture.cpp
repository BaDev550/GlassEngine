#include "gepch.h"
#include "Texture.h"
#include "Renderer.h"

#include "stb_image.h"

namespace ge::renderer {
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
		if (spec.attachment)
			createDesc.usageFlags |= utility::IsColorFormat(spec.format) ? ImageUsageFlagsBits::ColorAttachment : ImageUsageFlagsBits::DepthStencilAttachment;
		else
			createDesc.usageFlags |= ImageUsageFlagsBits::TransferDst;

		return ge::mem::Ref<Texture2D>::Create(Image::Create(createDesc), ImageSubresource{}, spec);
	}

	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpec& spec, const std::filesystem::path& filePath)
	{
		int width, height, channes;
		stbi_uc* data = stbi_load(filePath.string().c_str(), &width, &height, &channes, STBI_rgb_alpha);

		ImageSpec createDesc{};
		createDesc.imageFormat = spec.format;
		createDesc.extent = { spec.width, spec.height, 1 };
		createDesc.usageFlags |= ImageUsageFlagsBits::Readonly;
		if (spec.attachment)
			createDesc.usageFlags |= utility::IsColorFormat(spec.format) ?
			ImageUsageFlagsBits::ColorAttachment : ImageUsageFlagsBits::DepthStencilAttachment;
		else
			createDesc.usageFlags |= ImageUsageFlagsBits::TransferDst;

		auto texture = ge::mem::Ref<Texture2D>::Create(Image::Create(createDesc), ImageSubresource{}, spec);
		// Renderer3D::GetRenderAPI()->LoadDataToTexture2D({}, *texture, data, width * height * utility::GetPixelSize(spec.format));
		return texture;
	}

	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpec& spec, const void* data) {
		const auto* dataPtr = reinterpret_cast<const uint8_t*>(data);
		const auto dataSize = spec.width * spec.height * utility::GetPixelSize(spec.format);

		ImageSpec createDesc{};
		createDesc.imageFormat = spec.format;
		createDesc.extent = { spec.width, spec.height, 1 };
		createDesc.usageFlags |= ImageUsageFlagsBits::Readonly;
		if (spec.attachment)
			createDesc.usageFlags |= utility::IsColorFormat(spec.format) ?
			ImageUsageFlagsBits::ColorAttachment : ImageUsageFlagsBits::DepthStencilAttachment;
		else
			createDesc.usageFlags |= ImageUsageFlagsBits::TransferDst;

		auto texture = ge::mem::Ref<Texture2D>::Create(Image::Create(createDesc), ImageSubresource{}, spec);
		texture->_data = std::vector<uint8_t>(dataPtr, dataPtr + dataSize);
		// Renderer3D::GetRenderAPI()->LoadDataToTexture2D({}, *texture, data, dataSize);
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

		return ge::mem::Ref<Texture2D>(new Texture2D(image, subresource, {}));
	}
}
