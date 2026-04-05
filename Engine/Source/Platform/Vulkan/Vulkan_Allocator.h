#pragma once
#include <vma/vk_mem_alloc.h>
#include "GlassEngine/Memory/Allocator.h"

namespace ge::renderer::mem {
	class Vulkan_AllocatorCallbacks {
	public:
		static void InitCallbacks() {
			_allocationCallbacks.pUserData = nullptr;
			_allocationCallbacks.pfnAllocation = &Allocator;
			_allocationCallbacks.pfnReallocation = &Reallocation;
			_allocationCallbacks.pfnFree = &Free;
		}

		static inline VkAllocationCallbacks& GetCallbacks() {
			return _allocationCallbacks;
		}
		inline VmaDeviceMemoryCallbacks& GetDeviceCallbacks() {
			// TODO (0x): implement
		}
	private:
		static void* VKAPI_CALL Allocator(void* pUserData, size_t size, size_t aligment, VkSystemAllocationScope allocationScope) {
			return ge::mem::allocFuncs::GE_AllocateAligned(size, aligment);
		}
		static void* VKAPI_CALL Reallocation(void* pUserData, void* pOriginal, size_t size, size_t aligment, VkSystemAllocationScope allocationScope) {
			return ge::mem::allocFuncs::GE_ReallocateAligned(pOriginal, size, aligment);
		}
		static void  VKAPI_CALL Free(void* pUserData, void* pBlock) {
			ge::mem::allocFuncs::GE_FreeAligned(pBlock, 0, 0); // TODO (badev): pass actual size of object freed
		}
		static inline VkAllocationCallbacks _allocationCallbacks;
		static inline VmaDeviceMemoryCallbacks _deviceMemoryCallbacks;
	};

	class Vulkan_Allocator {
	public:
		Vulkan_Allocator() : _allocator(VK_NULL_HANDLE) {}
		~Vulkan_Allocator() {
			vmaDestroyAllocator(_allocator);
		}
		Vulkan_Allocator(const Vulkan_Allocator&) = delete;
		Vulkan_Allocator& operator=(const Vulkan_Allocator&) = delete;
		Vulkan_Allocator(Vulkan_Allocator&&) = delete;
		Vulkan_Allocator&& operator=(Vulkan_Allocator&&) = delete;

		void CreateAllocator(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& device) {
			VmaAllocatorCreateInfo createInfo{};
			createInfo.device = device;
			createInfo.instance = instance;
			createInfo.physicalDevice = physicalDevice;
			createInfo.pAllocationCallbacks = &Vulkan_AllocatorCallbacks::GetCallbacks();
			createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
			vmaCreateAllocator(&createInfo, &_allocator);
		}

		void AllocateBuffer(VkBuffer& buffer, VmaAllocation& allocation, VkBufferCreateInfo& createInfo, VmaMemoryUsage usage) {
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = usage;
			vmaCreateBuffer(_allocator, &createInfo, &allocInfo, &buffer, &allocation, nullptr);
		}

		void AllocateImage(VkImage& buffer, VmaAllocation& allocation, VkImageCreateInfo& createInfo, VmaMemoryUsage usage) {
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = usage;
			vmaCreateImage(_allocator, &createInfo, &allocInfo, &buffer, &allocation, nullptr);
		}
		VmaAllocator GetAllocator() const { return _allocator; }
	private:
		VmaAllocator _allocator;
	};
}