#include "gepch.h"
#include "Vulkan_Buffer.h"
#include "Vulkan_Types.h"

namespace ge::renderer {
	Vulkan_Buffer::Vulkan_Buffer(const BufferSpec& desc)
		: Buffer(desc)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.size = GetAlighnedElementSize() * desc.elementCount;
		bufferCreateInfo.usage = utility::Vulkan_GetBufferUsageFlags(_desc.usageFlags);
		bufferCreateInfo.flags = {};
		
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = utility::Vulkan_GetVMAMemoryUsage(_desc.memoryType);
		allocCreateInfo.flags = utility::Vulkan_GetVMAAllocFlags(_desc.cpuAccess);
		allocCreateInfo.requiredFlags = (_desc.cpuAccess != BufferCpuAccess::None) ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VkMemoryPropertyFlags{};

		VmaAllocationInfo allocInfo{};

		static_cast<Vulkan_RenderContext&>(_renderContext).GetAllocator().AllocateBuffer(bufferCreateInfo, allocCreateInfo, _buffer, _allocation, allocInfo);

		_mappedPtr = allocInfo.pMappedData;
	}

	Vulkan_Buffer::~Vulkan_Buffer() {
		VK_ALLOCATOR.DestroyBuffer(_buffer, _allocation);
	}

	void Vulkan_Buffer::SetDebugName(GEString name) const noexcept {
		VkDebugUtilsObjectNameInfoEXT nameInfo;
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectHandle = reinterpret_cast<uint64_t>(_buffer);
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.pObjectName = name.c_str();

		vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
	}
}