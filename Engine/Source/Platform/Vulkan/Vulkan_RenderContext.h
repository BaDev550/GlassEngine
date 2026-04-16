#pragma once
#include <vulkan/vulkan.h>

#include "GlassEngine/Renderer/RenderContext.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Core/memory.h"
#include "Vulkan_Allocator.h"
#include "Vulkan_Types.h"
#include <optional>
#include <span>
#include <array>

#define VK_RENDER_CONTEXT CastChecked<Vulkan_RenderContext>(&_renderContext)
#define VK_ALLOCATOR VK_RENDER_CONTEXT->GetAllocator()
#define VK_ALLOCATOR_CALLBACKS &mem::Vulkan_AllocatorCallbacks::GetCallbacks()

namespace ge::renderer {
	class Vulkan_BindlessManager;

	struct Vulkan_DeviceFeatures {
		bool unifiedImageLayouts;
		bool hostImageCopy;
		bool maintenance5;
		bool descriptorHeap;
	};

	class Vulkan_RenderContext final : public RenderContext {
	public:
		Vulkan_RenderContext(GLFWwindow* window);
		~Vulkan_RenderContext();
		virtual void Init() override;
		virtual void Wait() override;

		[[nodiscard]] mem::Vulkan_Allocator& GetAllocator() const noexcept { return *_allocator; }
		[[nodiscard]] VkInstance GetInstance() const noexcept { return _instance; }
		[[nodiscard]] VkPipelineCache GetPipelineCache() const noexcept { return _pipelineCache; }
		[[nodiscard]] VkDevice GetDevice() const noexcept { return _device; }
		[[nodiscard]] VkDescriptorPool GetGlobalDescriptorPool() const noexcept { return _globalDescriptorPool; }
		[[nodiscard]] Vulkan_DeviceFeatures GetDeviceFeatures() const noexcept { return _deviceFeatures; }

		[[nodiscard]] auto &GetBindlessManagersReadonlyImage() noexcept { return *_readonlyImageBindlessManager; }
		[[nodiscard]] auto &GetBindlessManagersWritableImage() noexcept { return *_writableImageBindlessManager; }
		[[nodiscard]] auto &GetBindlessManagersUniformBuffer() noexcept { return *_uniformBufferBindlessManager; }
		[[nodiscard]] auto &GetBindlessManagersSampler() noexcept { return *_samplerBindlessManager; }

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
		void CreateBindlessManagers();
		void CreateGlobalDescriptorPool();
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
		VkDescriptorPool _globalDescriptorPool = VK_NULL_HANDLE; // for user descriptors

		uint32_t _graphicsQueueFamilyIndex{};
		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		
		GLFWwindow* _window = nullptr;
		mem::Vulkan_Allocator* _allocator = nullptr;

		Vulkan_DeviceFeatures _deviceFeatures{};

		GEVector<const char*> _layers{};
		GEVector<const char*> _deviceExtensions{};

		std::array<ge::mem::Scope<Vulkan_BindlessManager>, 4> _bindlessManagers{};
		ge::mem::Scope<Vulkan_BindlessManager> _readonlyImageBindlessManager;
		ge::mem::Scope<Vulkan_BindlessManager> _writableImageBindlessManager;
		ge::mem::Scope<Vulkan_BindlessManager> _uniformBufferBindlessManager;
		ge::mem::Scope<Vulkan_BindlessManager> _samplerBindlessManager;

		std::array<VkSampleCountFlagBits, 4> _sampleMap{};
#ifdef _DEBUG
		const bool _useValidationLayer = true;
#else
		const bool _useValidationLayer = false;
#endif
	};
}