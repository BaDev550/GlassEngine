#pragma once

#include <GlassEngine/Utilities/Flags.h>
#include <GlassEngine/Utilities/Blob.h>
#include <GlassEngine/Core/Memory.h>

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

	enum class ShaderDataType : uint8_t {
		None = 0,
		Sampler2D,
		SamplerCube,
		UniformBuffer
	};

	enum class ShaderStageBits : uint16_t {
		Vertex = BIT(1),
		Geometry = BIT(2),
		TessellationControl = BIT(3),
		TessellationEvaluation = BIT(4),
		Fragment = BIT(5),
		Compute = BIT(6),
	};
	using ShaderStageFlags = ge::Flags<ShaderStageBits>;

	enum class ShaderResourceType : uint8_t {
		UniformBuffer = 0,
		ReadonlyBuffer,
		WritableBuffer,
		ReadonlyImage,
		WritableImage,
		Sampler,
	};

	enum class ShaderType : uint8_t {
		Graphics,
		Compute,
	};

	struct ShaderResource {
		GEString name;
		uint16_t resourceCount;
		uint16_t bindingIndex;
		ShaderResourceType type;
	};

	struct ShaderEntryPoint {
		GEString name;
		ShaderStageBits type;
	};

	struct ShaderReflection {
		std::unordered_map<GEString, ShaderEntryPoint> entryPoints;
		std::unordered_map<GEString, ShaderResource> resources;
	};

	struct ShaderData {
		ShaderReflection reflection;
		std::vector<char> byteCode;
	};

	namespace utility {
		[[nodiscard]] constexpr std::string_view ToString(ShaderResourceType type) {
			switch (type) {
			case ShaderResourceType::UniformBuffer:  return "UniformBuffer";
			case ShaderResourceType::ReadonlyBuffer: return "ReadonlyBuffer";
			case ShaderResourceType::WritableBuffer: return "WritableBuffer";
			case ShaderResourceType::ReadonlyImage:  return "ReadonlyImage";
			case ShaderResourceType::WritableImage:  return "WritableImage";
			case ShaderResourceType::Sampler:        return "Sampler";
			default:                                 return "Unknown";
			}
		}

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
struct ge::FlagTraits<ge::renderer::ShaderStageBits> {
	static constexpr bool is_bitmask = true;
};

template <>
struct ge::FlagTraits<ge::renderer::ImageUsageFlagsBits> {
	static constexpr bool is_bitmask = true;
};
