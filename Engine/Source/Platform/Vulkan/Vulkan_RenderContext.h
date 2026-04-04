#pragma once
#include <vulkan/vulkan.h>

#include "GlassEngine/Renderer/RenderContext.h"
#include "Vulkan_Allocator.h"
#include <optional>

namespace ge::renderer {
	struct QueueFamilies {
		std::optional<uint32_t> graphicsIndex;
		std::optional<uint32_t> presentIndex;

		bool IsValid() { return graphicsIndex.has_value() && presentIndex.has_value(); }
	};

	class Vulkan_RenderContext : public RenderContext {
	public:
		Vulkan_RenderContext(GLFWwindow* window);
		~Vulkan_RenderContext();
		virtual void Init() override;
	private:
		void CreateVulkanAllocator();
		void CreateInstance();
		void CreateDebugMessenger();
		void PickPhysicalDevice();
		void CreateSurface();
		void CreateLogicalDevice();
		bool CheckEnabledLayersSupport();
		GEVector<const char*> GetRequiredInstanceExtensions();
	private:
		VkInstance _instance = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties _physicalDeviceProperties;
		VkDevice _device;
		VkSurfaceKHR _surface;
		VkDebugUtilsMessengerEXT _debugMessenger;

		VkQueue _graphicsQueue;
		VkQueue _presentQueue;

		GLFWwindow*& _window;
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