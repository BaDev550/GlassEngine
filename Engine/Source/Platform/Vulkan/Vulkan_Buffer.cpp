#include "gepch.h"
#include "Vulkan_Buffer.h"

namespace ge::renderer {
	[[nodiscard]] constexpr VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
		if (minOffsetAlignment > 0)
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		return instanceSize;
	}

	[[nodiscard]] constexpr VkBufferUsageFlags GetBufferUsageFlags(BufferUsageFlags bufferUsageFlags) {
		VkBufferUsageFlags out{};
		if (bufferUsageFlags.Has(BufferUsageFlagsBits::Readonly_storage)) out |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Writable_storage)) out |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Transfer_dst)) out |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Transfer_src)) out |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Vertex)) out |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Index)) out |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		else if (bufferUsageFlags.Has(BufferUsageFlagsBits::Indirect)) out |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		return out;
	}

	[[nodiscard]] constexpr VmaAllocationCreateFlags GetVMAAllocFlags(BufferCpuAccess hostAccess) noexcept {
		VmaAllocationCreateFlags alloc_flags{};

		if (hostAccess != BufferCpuAccess::None)
			alloc_flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

		if (hostAccess == BufferCpuAccess::Write)
			alloc_flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

		else if (hostAccess == BufferCpuAccess::ReadWrite)
			alloc_flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		return alloc_flags;
	}

	[[nodiscard]] constexpr VmaMemoryUsage GetVMAMemoryUsage(BufferMemoryType memoryType) noexcept {
		switch (memoryType) {
			case BufferMemoryType::Auto: return VMA_MEMORY_USAGE_AUTO;
			case BufferMemoryType::DeviceMemory: return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			case BufferMemoryType::SystemMemory: return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		}
	}

	Vulkan_Buffer::Vulkan_Buffer(const BufferCreateDesc& desc)
		: Buffer(desc)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.size = GetAlighnedElementSize() * desc.elementCount;
		bufferCreateInfo.usage = GetBufferUsageFlags(_desc.usageFlags);
		bufferCreateInfo.flags = {};
		
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = GetVMAMemoryUsage(_desc.memoryType);
		allocCreateInfo.flags = GetVMAAllocFlags(_desc.cpuAccess);
		allocCreateInfo.requiredFlags = (_desc.cpuAccess != BufferCpuAccess::None) ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VkMemoryPropertyFlags{};

		VmaAllocationInfo allocInfo{};

		static_cast<Vulkan_RenderContext&>(_renderContext).GetAllocator().AllocateBuffer(bufferCreateInfo, allocCreateInfo, _buffer, _allocation, allocInfo);

		_mappedPtr = allocInfo.pMappedData;
	}

	Vulkan_Buffer::~Vulkan_Buffer() {
	}
}