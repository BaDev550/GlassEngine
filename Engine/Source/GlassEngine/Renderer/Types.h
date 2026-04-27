#pragma once

#include "GlassEngine/Utilities/Flags.h"
#include "GlassEngine/Utilities/Blob.h"
#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Core/Core.h"
#include <glm/glm.hpp>

namespace ge::renderer {
	enum class FilterType : uint8_t {
		Nearest,
		Linear,
	};

	enum class SamplerAddressMode : uint8_t {
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	enum class CompareOp : uint8_t {
		None,
		Never,
		Always,
		Equal,
		NotEqual,
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual,
	};

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
		RGBA8Unorm,
		// alias
		RGBA8 = RGBA8Unorm,
		RGBA8Srgb,
		RGBA8Int,
		RGBA8Uint,
		RGBA16Float,
		RGBA16Int,
		RGBA16Uint,
		RGBA32Float,
		RGBA32Int,
		RGBA32Uint,
		BC5Unorm,
		BC5Srgb,
		BC7Unorm,
		BC7Srgb,
		D16,
		D32,
		D32S8,
		D24S8,
	};

	// TODO (dnm): complate this
	enum class VertexFormat : uint8_t {
		RGBA32Float,
		RG32Float,
		R32Float,
		RGBA16Float,
		RG16Float,
		R16Float,
		RGBA32Int,
		RG32Int,
		R32Int,
		RGBA16Int,
		RG16Int,
		R16Int,
		RGBA32UInt,
		RG32UInt,
		R32UInt,
		RGBA16UInt,
		RG16UInt,
		R16UInt,
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
	
	struct ClearValue {
		ClearValue(float depth, uint8_t stencil) : depthClear(depth), stencilClear(stencil) {}
		ClearValue(glm::vec4 color) : colorClear(color) {}
		ClearValue() : colorClear(0,0,0,0) {}
		union {
			glm::vec4 colorClear;
			struct { float depthClear; uint8_t stencilClear; };
		};
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
		GEUnorderedMap<GEString, ShaderEntryPoint> entryPoints;
		GEUnorderedMap<GEString, ShaderResource> resources;
	};

	struct ShaderData {
		ShaderReflection reflection;
		GEVector<char> spirvByteCode;
		GEUnorderedMap<GEString, GEVector<char>> dxilByteCodes;
	};

	struct ImageSubresource {
		uint16_t baseLayer = 0;
		uint16_t layerCount = 1;
		uint8_t baseMipmap = 0;
		uint8_t mipmapCount = 1;
		ImageSubresourceType type = ImageSubresourceType::e2D;

		auto operator<=>(const ImageSubresource&) const = default;
	};

	enum class CullMode : uint8_t {
		None = 0,
		Back,
		Front
	};

	enum class DepthMode : uint8_t {
		None = 0,
		Less,
		LessOrEqual
	};

	enum class PrimitiveTopology : uint8_t {
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip
	};

	enum class PolygonMode : uint8_t {
		Fill,
		Wireframe
	};

	enum class VertexInputRate : uint8_t {
		Vertex,
		Instance
	};

	enum class StencilOp : uint8_t {
		Keep,
		Zero,
		Replace,
		IncramentAndClamp,
		DecrementAndClamp,
		Invert,
		IncramentAndWarp,
		DecrementAndWarp,
	};

	enum class AttachmentLoadOp : uint8_t {
		Load,
		Clear,
		None,
	};

	enum class AttachmentStoreOp : uint8_t {
		Store,
		None
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

		[[nodiscard]] constexpr uint32_t GetPixelSize(ImageFormat imageFormat) noexcept {
			switch (imageFormat) {
			case ImageFormat::RGBA8Unorm:  return 4;
			case ImageFormat::RGBA8Srgb:   return 4;
			case ImageFormat::RGBA8Int:    return 4;
			case ImageFormat::RGBA8Uint:   return 4;
			case ImageFormat::RGBA16Float: return 8;
			case ImageFormat::RGBA16Int:   return 8;
			case ImageFormat::RGBA16Uint:  return 8;
			case ImageFormat::RGBA32Float: return 16;
			case ImageFormat::RGBA32Int:   return 16;
			case ImageFormat::RGBA32Uint:  return 16;
			case ImageFormat::BC5Unorm:    return 1;
			case ImageFormat::BC5Srgb:     return 1;
			case ImageFormat::BC7Unorm:    return 1;
			case ImageFormat::BC7Srgb:     return 1;
			case ImageFormat::D16:         return 2;
			case ImageFormat::D32:         return 4;
			case ImageFormat::D32S8:       return 8;
			case ImageFormat::D24S8:       return 4;
			}
			return 0;
		}
		
		[[nodiscard]] constexpr uint32_t GetVertexSize(VertexFormat vertexFormat) noexcept {
			switch (vertexFormat) {
			case VertexFormat::RGBA32Float:  return 16;
			case VertexFormat::RG32Float:    return 8;
			case VertexFormat::R32Float:     return 4;

			case VertexFormat::RGBA16Float:  return 8;
			case VertexFormat::RG16Float:    return 4;
			case VertexFormat::R16Float:     return 2;

			case VertexFormat::RGBA32Int:    return 16;
			case VertexFormat::RG32Int:      return 8;
			case VertexFormat::R32Int:       return 4;

			case VertexFormat::RGBA16Int:    return 8;
			case VertexFormat::RG16Int:      return 4;
			case VertexFormat::R16Int:       return 2;

			case VertexFormat::RGBA32UInt:   return 16;
			case VertexFormat::RG32UInt:     return 8;
			case VertexFormat::R32UInt:      return 4;

			case VertexFormat::RGBA16UInt:   return 8;
			case VertexFormat::RG16UInt:     return 4;
			case VertexFormat::R16UInt:      return 2;
			}
			return 0;
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
