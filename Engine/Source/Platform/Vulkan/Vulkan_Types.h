#pragma once

#include "Vulkan_RenderContext.h"
#include <GlassEngine/Renderer/Types.h>
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
}