#pragma once
#include <Memory/Memory.h>

#include <vk_mem_alloc.h>

namespace ge::graphics {
	static std::string_view VkSystemAllocationScopeToString(VkSystemAllocationScope scope) {
		switch (scope)
		{
		case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: return "Vulkan_Allocator_Command";
		case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: return "Vulkan_Allocator_Object";
		case VK_SYSTEM_ALLOCATION_SCOPE_CACHE: return "Vulkan_Allocator_Cache";
		case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: return "Vulkan_Allocator_Device";
		case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: return "Vulkan_Allocator_Instance";
		default:
			return "Vulkan_Allocator";
		}
	}
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
			void* ptr = GE_PLATFORM_ALIGNED_ALLOC(size, aligment, VkSystemAllocationScopeToString(allocationScope).data());
			return ptr;
		}
		static void* VKAPI_CALL Reallocation(void* pUserData, void* pOriginal, size_t size, size_t aligment, VkSystemAllocationScope allocationScope) {
			return GE_PLATFORM_ALIGNED_REALLOC(pOriginal, size, aligment);
		}
		static void  VKAPI_CALL Free(void* pUserData, void* pBlock) {
			GE_PLATFORM_ALIGNED_FREE(pBlock, 0, 0);
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
			createInfo.vulkanApiVersion = VK_API_VERSION_1_4;
			createInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
				| VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
				| VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT
				| VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
			vmaCreateAllocator(&createInfo, &_allocator);
		}

		void AllocateBuffer(const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocCreateInfo, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo& allocInfo) {
			vmaCreateBuffer(_allocator, &createInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo);
		}

		void AllocateImage(const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocCreateInfo, VkImage& image, VmaAllocation& allocation, VmaAllocationInfo& allocInfo) {
			VkResult result = vmaCreateImage(_allocator, &createInfo, &allocCreateInfo, &image, &allocation, &allocInfo);
		}

		void DestroyBuffer(VkBuffer& buffer, VmaAllocation& allocation) {
			vmaDestroyBuffer(_allocator, buffer, allocation);
		}

		void DestroyImage(VkImage image, VmaAllocation allocation) {
			vmaDestroyImage(_allocator, image, allocation);
		}

		VmaAllocator GetAllocator() const { return _allocator; }
	private:
		VmaAllocator _allocator;
	};
}