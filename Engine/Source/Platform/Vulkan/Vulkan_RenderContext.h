#pragma once
#include <vulkan/vulkan.h>

#include "GlassEngine/Renderer/RenderContext.h"
#include "Vulkan_Allocator.h"
#include <optional>

namespace ge::renderer {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsIndex;
		std::optional<uint32_t> presentIndex;

		bool IsValid() { return graphicsIndex.has_value() && presentIndex.has_value(); }
	};

	class Vulkan_RenderContext : public RenderContext {
	public:
		Vulkan_RenderContext(GLFWwindow* window);
		~Vulkan_RenderContext();
		virtual void Init() override;
		virtual void Wait() override;

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& alloc);
		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImage& image, VmaAllocation& alloc);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		//void TransitionImageLayout(); later

		mem::Vulkan_Allocator& GetAllocator() { return *_allocator; }
		VkInstance GetInstance() const { return _instance; }
		VkDevice GetDevice() const { return _device; }
		VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
		VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const { return _physicalDeviceProperties; }
	private:
		void CreateVulkanAllocator();
		void CreateInstance();
		void CreateDebugMessenger();
		void PickPhysicalDevice();
		void CreateSurface();
		void CreateLogicalDevice();
		void CreateContextCommandPool();
		bool CheckEnabledLayersSupport();
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		GEVector<const char*> GetRequiredInstanceExtensions();

		VkCommandBuffer BeginSingleTimeCommand();
		void EndSingleTimeCommand(VkCommandBuffer cmd);
	private:
		VkInstance _instance = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties _physicalDeviceProperties;
		VkDevice _device = VK_NULL_HANDLE;
		VkSurfaceKHR _surface = VK_NULL_HANDLE;
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;

		VkQueue _graphicsQueue;
		VkQueue _presentQueue;

		GLFWwindow* _window;
		mem::Vulkan_Allocator* _allocator = nullptr;

		GEVector<const char*> _layers;
		GEVector<const char*> _deviceExtensions;
#ifdef _DEBUG
		bool _useValidationLayer = true;
#else
		bool _useValidationLayer = false;
#endif
	};
}