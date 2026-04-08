#pragma once
#include <vulkan/vulkan.h>

#include "GlassEngine/Renderer/RenderContext.h"
#include "GlassEngine/Core/Core.h"
#include "Vulkan_Allocator.h"
#include "Vulkan_Types.h"
#include <optional>

#define VK_RENDER_CONTEXT CastChecked<Vulkan_RenderContext>(&_renderContext)
#define VK_ALLOCATOR VK_RENDER_CONTEXT->GetAllocator()
#define VK_ALLOCATOR_CALLBACKS &mem::Vulkan_AllocatorCallbacks::GetCallbacks()

namespace ge::renderer {
	struct DeviceFeatures {
		bool unifiedImageLayouts;
		bool hostImageCopy;
		bool maintenance5;
		bool shaderStencilExport;
		bool memoryBudget;
		bool descriptorHeap;
		bool mutableDescriptorType;
	};

	class Vulkan_RenderContext final : public RenderContext {
	public:
		Vulkan_RenderContext(GLFWwindow* window);
		~Vulkan_RenderContext();
		virtual void Init() override;
		virtual void Wait() override;

		[[nodiscard]] mem::Vulkan_Allocator& GetAllocator() { return *_allocator; }
		[[nodiscard]] VkInstance GetInstance() const noexcept { return _instance; }
		[[nodiscard]] VkPipelineCache GetPipelineCache() const noexcept { return _pipelineCache; }
		[[nodiscard]] VkDevice GetDevice() const noexcept { return _device; }
		[[nodiscard]] DeviceFeatures GetDeviceFeatures() const noexcept { return _deviceFeatures; }

		[[nodiscard]] VkSampleCountFlagBits GetSampleCount(ImageSampleCount sample) const noexcept { return _sampleMap[static_cast<uint16_t>(sample)]; }
	private:
		void CreateVulkanAllocator();
		void CreateInstance();
		void CreateDebugMessenger();
		void PickPhysicalDevice();
		void CreateSurface();
		void CreateLogicalDevice();
		void CreateContextCommandPool();
		void CreatePipelineCache();
		bool CheckEnabledLayersSupport();
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
		void FindQueueFamilies();
		GEVector<const char*> GetRequiredInstanceExtensions();
		GEVector<const char*> GetRequiredDeviceExtensions();
		GEVector<const char*> GetSupportedOptionalDeviceExtensions();

		VkCommandBuffer BeginSingleTimeCommand();
		void EndSingleTimeCommand(VkCommandBuffer cmd);
	private:
		VkInstance _instance = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		VkDevice _device = VK_NULL_HANDLE;
		VkSurfaceKHR _surface = VK_NULL_HANDLE;
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
		VkPipelineCache _pipelineCache = VK_NULL_HANDLE;

		uint32_t _graphicsQueueFamilyIndex{};
		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		
		GLFWwindow* _window = nullptr;
		mem::Vulkan_Allocator* _allocator = nullptr;

		DeviceFeatures _deviceFeatures{};

		GEVector<const char*> _layers{};
		GEVector<const char*> _deviceExtensions{};

		std::array<VkSampleCountFlagBits, 4> _sampleMap;
#ifdef _DEBUG
		const bool _useValidationLayer = true;
#else
		const bool _useValidationLayer = false;
#endif
	};
}