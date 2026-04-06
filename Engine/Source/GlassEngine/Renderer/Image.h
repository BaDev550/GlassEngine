#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "RenderObject.h"
#include <glm/glm.hpp>
#include <bitset>

namespace ge::renderer {
	enum class ImageUsageFlagsBits : uint16_t {
		None = 0,
		Readonly = BIT(0),
		Writable = BIT(1),
		TransferDst = BIT(2),
		TransferSrc = BIT(3),
		ColorAttachment = BIT(4),
		DepthStencilAttachment = BIT(5)
	};
	using ImageUsageFlags = ge::Flags<ImageUsageFlagsBits>;

	enum class ImageSampleCount : uint8_t {
		e1 = 0,
		e2,
		e4,
		e8
	};

	enum class ImageType : uint8_t {
		e1D = 0,
		e2D,
		e3D
	};

	enum class ImageFormat : uint8_t {
		eRGBA8 = 0
	};

	enum class ImageFilter : uint8_t {
		Linear,
		Nearest
	};

	enum class ImageSubresourceType : uint8_t {
		e1D = 0,
		e1DArray,
		e2D,
		e2DArray,
		e3D,
		eCube,
		eCubeArray,
	};

	struct ImageCreateDesc {
		glm::uvec3 extent;
		ImageUsageFlags usageFlags;
		ImageSampleCount sampleCount;
		ImageFormat imageFormat;
		ImageType imageType;
		uint16_t arrayCount;
		uint16_t mipmapCount;
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
		[[nodiscard]] static ge::mem::Ref<Image> Create(const ImageCreateDesc& desc);

		explicit Image(const ImageCreateDesc& desc) noexcept : _desc(desc) {}
		virtual ~Image() = default;

		[[nodiscard]] const auto& GetDescRef() const noexcept { return _desc; }
		[[nodiscard]] auto GetDesc() const noexcept { return _desc; }
	protected:
		ImageCreateDesc _desc;
	};
}

template <>
struct ge::FlagTraits<ge::renderer::ImageUsageFlagsBits> {
	static constexpr bool is_bitmask = true;
};

namespace std {
	template <>
	struct hash<ge::renderer::ImageSubresource> {
		size_t operator()(const ge::renderer::ImageSubresource subresource) const {
			return std::hash<uint64_t>{}(std::bit_cast<uint64_t>(subresource));
		}
	};
}