#pragma once
#include <vulkan/vulkan.h>

#include "GlassEngine/Renderer/RenderContext.h"
#include "GlassEngine/Core/Core.h"
#include "Vulkan_Allocator.h"
#include <GlassEngine/Renderer/Types.h>
#include <optional>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace ge::renderer::utility {
	[[nodiscard]] constexpr VkBufferUsageFlags Vulkan_GetBufferUsageFlags(BufferUsageFlags bufferUsageFlags) noexcept {
		VkBufferUsageFlags out{};
		if (bufferUsageFlags.Has(BufferUsageFlagsBits::Readonly)) out |= (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Writable)) out |= (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::TransferDst)) out |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::TransferSrc)) out |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Uniform)) out |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Vertex)) out |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Index)) out |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Indirect)) out |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		return out;
	}

	[[nodiscard]] constexpr VmaAllocationCreateFlags Vulkan_GetVMAAllocFlags(BufferCpuAccess hostAccess) noexcept {
		VmaAllocationCreateFlags alloc_flags{};

		if (hostAccess != BufferCpuAccess::None)
			alloc_flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

		if (hostAccess == BufferCpuAccess::Write)
			alloc_flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

		else if (hostAccess == BufferCpuAccess::ReadWrite)
			alloc_flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		return alloc_flags;
	}

	[[nodiscard]] constexpr VmaMemoryUsage Vulkan_GetVMAMemoryUsage(BufferMemoryType memoryType) noexcept {
		switch (memoryType) {
		case BufferMemoryType::Auto: return VMA_MEMORY_USAGE_AUTO;
		case BufferMemoryType::DeviceMemory: return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		case BufferMemoryType::SystemMemory: return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		}
	}

	[[nodiscard]] constexpr VkImageAspectFlags Vulkan_GetAspectFlags(ImageFormat imageFormat) noexcept {
		if (IsColorFormat(imageFormat)) return VK_IMAGE_ASPECT_COLOR_BIT;
		else if (IsDepthFormat(imageFormat)) return VK_IMAGE_ASPECT_DEPTH_BIT;
		else if (IsDepthStencilFormat(imageFormat)) return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		return {};
		
	}

	[[nodiscard]] constexpr VkFormat Vulkan_GetImageFormat(ImageFormat imageFormat) noexcept {
		switch (imageFormat) {
			case ImageFormat::RGBA8Unorm:  		return VK_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::RGBA8Srgb:   		return VK_FORMAT_R8G8B8A8_SRGB;
			case ImageFormat::RGBA8Int:    		return VK_FORMAT_R8G8B8A8_SINT;
			case ImageFormat::RGBA8Uint:   		return VK_FORMAT_R8G8B8A8_UINT;
			case ImageFormat::RGBA16Float: 		return VK_FORMAT_R16G16B16A16_SFLOAT;
			case ImageFormat::RGBA16Int:   		return VK_FORMAT_R16G16B16A16_SINT;
			case ImageFormat::RGBA16Uint:  		return VK_FORMAT_R16G16B16A16_UINT;
			case ImageFormat::RGBA32Float: 		return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ImageFormat::RGBA32Int:   		return VK_FORMAT_R32G32B32A32_SINT;
			case ImageFormat::RGBA32Uint:  		return VK_FORMAT_R32G32B32A32_UINT;
			case ImageFormat::BC3Unorm:    		return VK_FORMAT_BC3_UNORM_BLOCK;
			case ImageFormat::BC3Srgb:     		return VK_FORMAT_BC3_SRGB_BLOCK;
			case ImageFormat::BC5Unorm:    		return VK_FORMAT_BC5_UNORM_BLOCK;
			case ImageFormat::BC5Srgb:     		return VK_FORMAT_BC5_SNORM_BLOCK;
			case ImageFormat::BC7Unorm:    		return VK_FORMAT_BC7_UNORM_BLOCK;
			case ImageFormat::BC7Srgb:     		return VK_FORMAT_BC7_SRGB_BLOCK;
			case ImageFormat::D16:         		return VK_FORMAT_D16_UNORM;
			case ImageFormat::D32:         		return VK_FORMAT_D32_SFLOAT;
			case ImageFormat::D32S8:       		return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case ImageFormat::D24S8:       		return VK_FORMAT_D24_UNORM_S8_UINT;
			case ImageFormat::RG8Uint:			return VK_FORMAT_R8G8_UINT;
			case ImageFormat::RG8Unorm:			return VK_FORMAT_R8G8_UNORM;
			case ImageFormat::R10G10B10A2Unorm:	return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			case ImageFormat::R8Uint:			return VK_FORMAT_R8_UINT;
			case ImageFormat::R8Unorm:			return VK_FORMAT_R8_UNORM;
			case ImageFormat::R32Uint:			return VK_FORMAT_R32_UINT;
				break;
			}
		return {};
	}
	
	[[nodiscard]] constexpr VkFormat Vulkan_GetVertexFormat(VertexFormat vertexFormat) noexcept {
		switch (vertexFormat) {
		case VertexFormat::RGBA32Float:  return VK_FORMAT_R32G32B32A32_SFLOAT;
		case VertexFormat::RG32Float:    return VK_FORMAT_R32G32_SFLOAT;
		case VertexFormat::R32Float:     return VK_FORMAT_R32_SFLOAT;

		case VertexFormat::RGBA16Float:  return VK_FORMAT_R16G16B16A16_SFLOAT;
		case VertexFormat::RG16Float:    return VK_FORMAT_R16G16_SFLOAT;
		case VertexFormat::R16Float:     return VK_FORMAT_R16_SFLOAT;

		case VertexFormat::RGBA32Int:    return VK_FORMAT_R32G32B32A32_SINT;
		case VertexFormat::RG32Int:      return VK_FORMAT_R32G32_SINT;
		case VertexFormat::R32Int:       return VK_FORMAT_R32_SINT;

		case VertexFormat::RGBA16Int:    return VK_FORMAT_R16G16B16A16_SINT;
		case VertexFormat::RG16Int:      return VK_FORMAT_R16G16_SINT;
		case VertexFormat::R16Int:       return VK_FORMAT_R16_SINT;

		case VertexFormat::RGBA32UInt:   return VK_FORMAT_R32G32B32A32_UINT;
		case VertexFormat::RG32UInt:     return VK_FORMAT_R32G32_UINT;
		case VertexFormat::R32UInt:      return VK_FORMAT_R32_UINT;

		case VertexFormat::RGBA16UInt:   return VK_FORMAT_R16G16B16A16_UINT;
		case VertexFormat::RG16UInt:     return VK_FORMAT_R16G16_UINT;
		case VertexFormat::R16UInt:      return VK_FORMAT_R16_UINT;
		}
		return VK_FORMAT_UNDEFINED;
	}
	
	[[nodiscard]] constexpr VkImageType Vulkan_GetImageType(ImageType imageType) noexcept {
		switch (imageType) {
		case ImageType::e1D: return VK_IMAGE_TYPE_1D;
		case ImageType::e2D: return VK_IMAGE_TYPE_2D;
		case ImageType::e3D: return VK_IMAGE_TYPE_3D;
		}
	}

	[[nodiscard]] constexpr VkImageUsageFlags Vulkan_GetImageUsageFlags(ImageUsageFlags usageFlags) noexcept {
		VkImageUsageFlags out{};

		if (usageFlags.Has(ImageUsageFlagsBits::ColorAttachment))
			out |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::DepthStencilAttachment))
			out |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::Readonly))
			out |= VK_IMAGE_USAGE_SAMPLED_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::Writable))
			out |= VK_IMAGE_USAGE_STORAGE_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::TransferSrc))
			out |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if (usageFlags.Has(ImageUsageFlagsBits::TransferDst))
			out |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		return out;
	}

	[[nodiscard]] constexpr VkDescriptorType ShaderReflectionTypeToVulkanType(const ShaderDataType& type) noexcept {
		switch (type)
		{
		case ShaderDataType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case ShaderDataType::Sampler2D: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case ShaderDataType::SamplerCube: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		default: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
	}

	[[nodiscard]] constexpr VkDescriptorType Vulkan_GetDescriptorType(ShaderResourceType type) noexcept {
		switch (type) {
		case ShaderResourceType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case ShaderResourceType::ReadonlyImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case ShaderResourceType::WritableImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case ShaderResourceType::ReadonlyBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case ShaderResourceType::WritableBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case ShaderResourceType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
		}
	}

	[[nodiscard]] constexpr VkFilter Vulkan_GetFilterType(FilterType type) noexcept {
		switch (type) {
		case FilterType::Linear: return VK_FILTER_LINEAR;
		case FilterType::Nearest: return VK_FILTER_NEAREST;
		}
	}

	[[nodiscard]] constexpr VkSamplerMipmapMode Vulkan_GetMipmapFilterType(FilterType type) noexcept {
		switch (type) {
		case FilterType::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case FilterType::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}
	}

	[[nodiscard]] constexpr VkSamplerAddressMode Vulkan_GetSamplerAddressMode(SamplerAddressMode mode) noexcept {
		switch (mode) {
		case SamplerAddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case SamplerAddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case SamplerAddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	}

	[[nodiscard]] constexpr VkCompareOp Vulkan_GetCompareOp(CompareOp op) noexcept {
		switch (op) {
		case CompareOp::None:          return VK_COMPARE_OP_ALWAYS;
		case CompareOp::Never:          return VK_COMPARE_OP_NEVER;
		case CompareOp::Always:         return VK_COMPARE_OP_ALWAYS;
		case CompareOp::Equal:          return VK_COMPARE_OP_EQUAL;
		case CompareOp::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
		case CompareOp::Less:           return VK_COMPARE_OP_LESS;
		case CompareOp::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CompareOp::Greater:        return VK_COMPARE_OP_GREATER;
		case CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
		}
	}

	[[nodiscard]] constexpr VkImageLayout Vulkan_OptimalImageLayout(ImageUsageFlags flags, bool depthWrite = false, bool stencilWrite = false) noexcept {
		if (flags.Has(ImageUsageFlagsBits::Readonly) && !flags.Has(ImageUsageFlagsBits::DepthStencilAttachment)) {
			if (flags.Has(ImageUsageFlagsBits::Writable))
				return VK_IMAGE_LAYOUT_GENERAL;
			else
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		if (!flags.Has(ImageUsageFlagsBits::Writable)) {
			// ordering is neccesery
			if (flags.Has(ImageUsageFlagsBits::ColorAttachment))
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			else if (flags.Has(ImageUsageFlagsBits::DepthStencilAttachment)) {
				if (depthWrite && stencilWrite)
					return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				else if (depthWrite)
					return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
				else if (stencilWrite)
					return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
				else
					return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}

			else if (flags.Has(ImageUsageFlagsBits::TransferDst))
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			else if (flags.Has(ImageUsageFlagsBits::TransferSrc))
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
		return VK_IMAGE_LAYOUT_GENERAL;
	}

	[[nodiscard]] constexpr VkImageViewType Vulkan_ImageViewType(ImageSubresourceType type) noexcept {
		switch (type) {
		case ImageSubresourceType::e1D: return VK_IMAGE_VIEW_TYPE_1D;
		case ImageSubresourceType::e1DArray: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		case ImageSubresourceType::e2D: return VK_IMAGE_VIEW_TYPE_2D;
		case ImageSubresourceType::e2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		case ImageSubresourceType::e3D: return VK_IMAGE_VIEW_TYPE_3D;
		case ImageSubresourceType::eCube: return VK_IMAGE_VIEW_TYPE_CUBE;
		case ImageSubresourceType::eCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		}
	}

	[[nodiscard]] constexpr VkVertexInputRate Vulkan_GetVertexInputRate(VertexInputRate inputRate) noexcept {
		switch (inputRate) {
		case VertexInputRate::Vertex: return VK_VERTEX_INPUT_RATE_VERTEX;
		case VertexInputRate::Instance: return VK_VERTEX_INPUT_RATE_INSTANCE;
		}
	}

	[[nodiscard]] constexpr VkPrimitiveTopology Vulkan_GetPrimitiveTopology(PrimitiveTopology topology) noexcept {
		switch (topology) {
		case PrimitiveTopology::PointList:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case PrimitiveTopology::LineList:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case PrimitiveTopology::LineStrip:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case PrimitiveTopology::TriangleList:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
	}

	[[nodiscard]] constexpr VkPolygonMode Vulkan_GetPolygonMode(PolygonMode polygonMode) noexcept {
		switch (polygonMode) {
		case PolygonMode::Fill:      return VK_POLYGON_MODE_FILL;
		case PolygonMode::Wireframe: return VK_POLYGON_MODE_LINE;
		}
	}

	[[nodiscard]] constexpr VkCullModeFlags Vulkan_GetCullMode(CullMode cullMode) noexcept {
		switch (cullMode) {
		case CullMode::None:  return VK_CULL_MODE_NONE;
		case CullMode::Back:  return VK_CULL_MODE_BACK_BIT;
		case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
		}
	}

	[[nodiscard]] constexpr VkStencilOp Vulkan_GetStencilOp(StencilOp stencilOp) noexcept {
		switch (stencilOp) {
		case StencilOp::Keep:              return VK_STENCIL_OP_KEEP;
		case StencilOp::Zero:              return VK_STENCIL_OP_ZERO;
		case StencilOp::Replace:           return VK_STENCIL_OP_REPLACE;
		case StencilOp::IncramentAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case StencilOp::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case StencilOp::Invert:            return VK_STENCIL_OP_INVERT;
		case StencilOp::IncramentAndWarp:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case StencilOp::DecrementAndWarp:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		}
	}

	[[nodiscard]] constexpr VkAttachmentLoadOp Vulkan_GetLoadOp(AttachmentLoadOp op) noexcept {
		switch (op) {
		case AttachmentLoadOp::Load:              return VK_ATTACHMENT_LOAD_OP_LOAD;
		case AttachmentLoadOp::Clear:             return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case AttachmentLoadOp::None:              return VK_ATTACHMENT_LOAD_OP_NONE;
		}
	}

	[[nodiscard]] constexpr VkAttachmentStoreOp Vulkan_GetStoreOp(AttachmentStoreOp op) noexcept {
		switch (op) {
		case AttachmentStoreOp::Store:              return VK_ATTACHMENT_STORE_OP_STORE;
		case AttachmentStoreOp::None:              return VK_ATTACHMENT_STORE_OP_NONE;
		}
	}
}