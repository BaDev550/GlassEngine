#pragma once
#include <vulkan/vulkan.h>

#include "GlassEngine/Renderer/RenderContext.h"
#include "GlassEngine/Core/Core.h"
#include "Vulkan_Allocator.h"
#include <GlassEngine/Renderer/Types.h>
#include <optional>
#include <utility>

namespace ge::renderer::utility {
	[[nodiscard]] constexpr VkBufferUsageFlags Vulkan_GetBufferUsageFlags(BufferUsageFlags bufferUsageFlags) noexcept {
		VkBufferUsageFlags out{};
		if (bufferUsageFlags.Has(BufferUsageFlagsBits::Readonly)) out |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Writable)) out |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::TransferDst)) out |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::TransferSrc)) out |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
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
		if (IsDepthFormat(imageFormat)) return VK_IMAGE_ASPECT_COLOR_BIT;
		else if (IsDepthFormat(imageFormat)) return VK_IMAGE_ASPECT_DEPTH_BIT;
		else if (IsDepthStencilFormat(imageFormat)) return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		return {};
	}

	[[nodiscard]] constexpr VkFormat Vulkan_GetImageFormat(ImageFormat imageFormat) noexcept {
		switch (imageFormat) {
		case ImageFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
		}
	}

	[[nodiscard]] constexpr VkFormat Vulkan_GetVertexFormat(VertexFormat imageFormat) noexcept {
		switch (imageFormat) {
		case VertexFormat::RGBA32f: return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
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

	[[nodiscard]] constexpr uint32_t GetFormatSize(VkFormat format) noexcept {
		switch (format)
		{
		case VK_FORMAT_UNDEFINED: return 0;
		case VK_FORMAT_R8G8B8A8_UNORM: return 4;
		case VK_FORMAT_R8G8B8A8_SRGB: return 4;
		case VK_FORMAT_R16_SFLOAT: return 2;
		case VK_FORMAT_R16G16_SFLOAT: return 4;
		case VK_FORMAT_R16G16B16_SFLOAT: return 8;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return 16;
		case VK_FORMAT_R32_SFLOAT: return 4;
		case VK_FORMAT_R32G32_SFLOAT: return 8;
		case VK_FORMAT_R32G32B32_SFLOAT: return 12;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
		}
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

	[[nodiscard]] constexpr VkImageLayout Vulkan_OptimalImageLayout(ImageUsageFlags flags) noexcept {
		if (flags.Has(ImageUsageFlagsBits::Readonly)) {
			if (flags.Has(ImageUsageFlagsBits::Writable))
				return VK_IMAGE_LAYOUT_GENERAL;
			else
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		if (!flags.Has(ImageUsageFlagsBits::Writable))
		{
			// ordering is neccesery
			if (flags.Has(ImageUsageFlagsBits::ColorAttachment))
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			else if (flags.Has(ImageUsageFlagsBits::DepthStencilAttachment))
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			else if (flags.Has(ImageUsageFlagsBits::TransferDst))
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			else if (flags.Has(ImageUsageFlagsBits::TransferSrc))
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
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
}