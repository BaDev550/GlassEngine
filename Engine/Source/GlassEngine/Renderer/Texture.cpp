#include "gepch.h"
#include "Texture.h"
#include "Renderer.h"

#include "stb_image.h"

namespace ge::renderer {
	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpec& spec)
	{
		ImageSpec createDesc{};
		createDesc.imageFormat = spec.format;
		createDesc.extent = { spec.width, spec.height, 1 };
		createDesc.usageFlags |= ImageUsageFlagsBits::Readonly;
		if (spec.attachment)
			createDesc.usageFlags |= utility::IsColorFormat(spec.format) ?
			ImageUsageFlagsBits::ColorAttachment : ImageUsageFlagsBits::DepthStencilAttachment;
		else
			createDesc.usageFlags |= ImageUsageFlagsBits::TransferDst;

		return ge::mem::Ref<Texture2D>(new Texture2D(Image::Create(createDesc), spec));
	}

<<<<<<< Updated upstream
	ge::mem::Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec, const void* data)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Texture2D>::Create(spec, data);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
=======
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

		auto texture = ge::mem::Ref<Texture2D>(new Texture2D(Image::Create(createDesc), spec));
		// Renderer3D::GetRenderAPI()->LoadDataToTexture2D({}, *texture, data, width * height * utility::GetPixelSize(spec.format));
		return texture;
>>>>>>> Stashed changes
	}
}