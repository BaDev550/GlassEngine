#pragma once

#include "GlassEngine/Utilities/Flags.h"

namespace ge::renderer {
	enum class BufferUsageFlagsBits : uint16_t {
		None = 0,
		Uniform = BIT(0),
		Readonly = BIT(1),
		Writable = BIT(2),
		TransferDst = BIT(3),
		TransferSrc = BIT(4),
		Vertex = BIT(5),
		Index = BIT(6),
		Indirect = BIT(7),
	};
	using BufferUsageFlags = ge::Flags<BufferUsageFlagsBits>;

	enum class BufferCpuAccess : uint8_t {
		None = 0,
		Write,
		ReadWrite
	};

	enum class BufferMemoryType : uint8_t {
		Auto = 0,
		DeviceMemory,
		SystemMemory
	};

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

	// TODO (dnm): complate this
	enum class ImageFormat : uint8_t {
		RGBA8 = 0,
		D16,
		D32,
		D32S8,
		D24S8,
	};

	// TODO (dnm): complate this
	enum class VertexFormat : uint8_t {
		RGBA32f = 0
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

	namespace utility {
		[[nodiscard]] constexpr bool IsDepthFormat(ImageFormat imageFormat) noexcept {
			return ImageFormat::D16 == imageFormat || ImageFormat::D32 == imageFormat;
		}

		[[nodiscard]] constexpr bool IsDepthStencilFormat(ImageFormat imageFormat) noexcept {
			return ImageFormat::D24S8 == imageFormat || ImageFormat::D32S8 == imageFormat;
		}

		[[nodiscard]] constexpr bool IsColorFormat(ImageFormat imageFormat) noexcept {
			return !(IsDepthFormat(imageFormat) || IsDepthStencilFormat(imageFormat));
		}
	}
}

template <>
struct ge::FlagTraits<ge::renderer::BufferUsageFlagsBits> {
	static constexpr bool is_bitmask = true;
};

template <>
struct ge::FlagTraits<ge::renderer::ImageUsageFlagsBits> {
	static constexpr bool is_bitmask = true;
};
