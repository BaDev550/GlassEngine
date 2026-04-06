#include "gepch.h"
#include "Vulkan_Texture.h"

#include "stb_image.h"

namespace ge::renderer {
	Vulkan_Texture2D::Vulkan_Texture2D(const TextureSpecification& spec) {
		_specs = spec;
	}

	Vulkan_Texture2D::Vulkan_Texture2D(const TextureSpecification& spec, const std::filesystem::path& filePath) {
		_specs = spec;
		int width, height, channes;
		stbi_uc* data = stbi_load(filePath.string().c_str(), &width, &height, &channes, STBI_rgb_alpha);

		if (data) {
			ImageCreateDesc createDesc{};
			createDesc.imageFormat = _specs.format;
			createDesc.extent = { _specs.width, _specs.height, 1 };
			createDesc.usageFlags |= ImageUsageFlagsBits::Readonly;
			if (_specs.attachment)
				createDesc.usageFlags |= utility::IsColorFormat(_specs.format) ?
					ImageUsageFlagsBits::ColorAttachment : ImageUsageFlagsBits::DepthStencilAttachment;
			else
				createDesc.usageFlags |= ImageUsageFlagsBits::TransferDst;

			_image = Image::Create(createDesc); // TODO (0x): add a function to pass data into image
		}
	}
}