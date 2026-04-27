#pragma once

#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "RenderObject.h"
#include "Types.h"

#include <glm/glm.hpp>
#include <bitset>

namespace ge::renderer {
	struct ImageSpec {
		glm::uvec3 extent {1,1,1};
		ImageUsageFlags usageFlags;
		ImageSampleCount sampleCount = ImageSampleCount::e1;
		ImageFormat imageFormat;
		ImageType imageType = ImageType::e2D;
		uint16_t arrayCount = 1;
		uint16_t mipmapCount = 1;
	};

	class Image : public RenderObject {
	public:
		[[nodiscard]] static ge::mem::Ref<Image> Create(const ImageSpec& desc);

		explicit Image(const ImageSpec& desc) noexcept : _desc(desc) {}
		virtual ~Image() = default;
		
		[[nodiscard]] const auto& GetSpecRef() const noexcept { return _desc; }
		[[nodiscard]] auto GetSpec() const noexcept { return _desc; }
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