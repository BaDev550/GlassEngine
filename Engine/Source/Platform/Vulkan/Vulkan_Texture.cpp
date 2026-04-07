#include "gepch.h"
#include "Vulkan_Texture.h"

namespace ge::renderer {
	Vulkan_Texture2D::Vulkan_Texture2D(const TextureSpecification& spec) {
		_specs = spec;
	}

	Vulkan_Texture2D::Vulkan_Texture2D(const TextureSpecification& spec, const void* data) {
		_specs = spec;
		size_t size = _specs.width * _specs.height * 4; // for rgba
		if (data) {
			_data.resize(size);
			memcpy(_data.data(), data, size);
			ImageCreateDesc createDesc{};
			createDesc.imageFormat = _specs.format;
			createDesc.extent = { _specs.width, _specs.height, 1 };
			createDesc.usageFlags = ImageUsageFlagsBits::ColorAttachment;
			_image = Image::Create(createDesc); // TODO (0x): add a function to pass data into image
		}
	}
}