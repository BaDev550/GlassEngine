#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "RenderObject.h"
#include "Types.h"

#include <glm/glm.hpp>
#include <bitset>

namespace ge::renderer {
	struct ImageSpec {
		glm::uvec3 extent;
		ImageUsageFlags usageFlags;
		ImageSampleCount sampleCount = ImageSampleCount::e1;
		ImageFormat imageFormat;
		ImageType imageType = ImageType::e2D;
		uint16_t arrayCount = 1;
		uint16_t mipmapCount = 1;
	};

	struct ImageSubresource {
		uint16_t baseLayer;
		uint16_t LayerCount;
		uint8_t baseMipmap;
		uint8_t mipmapCount;
		ImageSubresourceType imageSubresourceType;

		auto operator<=>(const ImageSubresource&) const = default;
	};

	class Image : public RenderObject {
	public:
		[[nodiscard]] static ge::mem::Ref<Image> Create(const ImageSpec& desc);

		explicit Image(const ImageSpec& desc) noexcept : _desc(desc) {}
		virtual ~Image() = default;

		[[nodiscard]] const auto& GetDescRef() const noexcept { return _desc; }
		[[nodiscard]] auto GetDesc() const noexcept { return _desc; }
	protected:
		ImageSpec _desc;
	};
}

namespace std {
	template <>
	struct hash<ge::renderer::ImageSubresource> {
		size_t operator()(const ge::renderer::ImageSubresource subresource) const {
			return std::hash<uint64_t>{}(std::bit_cast<uint64_t>(subresource));
		}
	};
}