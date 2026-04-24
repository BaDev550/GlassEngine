#pragma once
#include <vulkan/vulkan.h>

#include "GlassEngine/Renderer/RenderContext.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Memory/Memory.h"
#include "Vulkan_Allocator.h"
#include "Vulkan_Types.h"
#include <optional>
#include <span>
#include <array>

#define VK_RENDER_CONTEXT CastChecked<Vulkan_RenderContext>(&_renderContext)
#define VK_ALLOCATOR VK_RENDER_CONTEXT->GetAllocator()
#define VK_ALLOCATOR_CALLBACKS &::ge::renderer::mem::Vulkan_AllocatorCallbacks::GetCallbacks()
#define VK_DEFINE_EXT_FUNC(name) inline PFN_##name name

namespace ge::renderer {
	class Vulkan_DescriptorManager;
	class Vulkan_Image;
	class Vulkan_Buffer;
	class Vulkan_Sampler;

	VK_DEFINE_EXT_FUNC(vkSetDebugUtilsObjectNameEXT);
	VK_DEFINE_EXT_FUNC(vkCreateDebugUtilsMessengerEXT);
	VK_DEFINE_EXT_FUNC(vkDestroyDebugUtilsMessengerEXT);
	VK_DEFINE_EXT_FUNC(vkCmdBeginDebugUtilsLabelEXT);
	VK_DEFINE_EXT_FUNC(vkCmdEndDebugUtilsLabelEXT);

	struct Vulkan_DeviceFeatures {
		bool unifiedImageLayouts;
		bool hostImageCopy;
		bool maintenance5;
		bool descriptorHeap;

		bool discrateGpu;
		bool umaOrRebar;
		
		bool partiallyBoundForSampler; // TODO (dnm): maybe has performance impact, need to test
		bool partiallyBoundForSampledImage; // TODO (dnm): maybe has performance impact, need to test
		bool partiallyBoundForStorageImage; // TODO (dnm): maybe has performance impact, need to test
	};

	constexpr uint32_t VULKAN_SAMPLED_IMAGE_COUNT = 200'000;
	constexpr uint32_t VULKAN_STORAGE_IMAGE_COUNT = 20'000;
	constexpr uint32_t VULKAN_SAMPLER_COUNT = 1'000;

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
		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const noexcept { return _physicalDevice; }
		[[nodiscard]] VkSurfaceKHR GetSurface() const noexcept { return _surface; }
		[[nodiscard]] Vulkan_DeviceFeatures GetDeviceFeatures() const noexcept { return _deviceFeatures; }
		[[nodiscard]] VkQueue GetGraphicsQueue() const noexcept { return _graphicsQueue; }
		[[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const noexcept { return _graphicsQueueFamilyIndex; }

		[[nodiscard]] virtual uint32_t IGetReadonlyImageHandle(Image& image, ImageSubresource subresource) override;
		[[nodiscard]] virtual uint32_t IGetWritableImageHandle(Image& image, ImageSubresource subresource) override;
		[[nodiscard]] virtual uint32_t IGetSamplerHandle(Sampler& sampler) override;

		void BindDescriptorSets(VkCommandBuffer commandBuffer) const noexcept;

		[[nodiscard]] auto &GetDescriptorManager() noexcept { return *_descriptorManager; }

		[[nodiscard]] VkSampleCountFlagBits GetSampleCount(ImageSampleCount sample) const noexcept { return _sampleMap[static_cast<uint16_t>(sample)]; }
	private:
		void CreateVulkanAllocator();
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateSurface();
		void CreateLogicalDevice();
		void CreateContextCommandPool();
		void CreatePipelineCache();
		bool CheckEnabledLayersSupport();
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
		void FindQueueFamilies();
		void CreateDescriptorManager();
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

		ge::mem::Scope<Vulkan_DescriptorManager> _descriptorManager;

		uint32_t _graphicsQueueFamilyIndex{};
		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		
		GLFWwindow* _window = nullptr;
		mem::Vulkan_Allocator* _allocator = nullptr;

		Vulkan_DeviceFeatures _deviceFeatures{};

		GEVector<const char*> _layers{};
		GEVector<const char*> _deviceExtensions{};

		std::array<VkSampleCountFlagBits, 4> _sampleMap{};
#ifdef _DEBUG
		const bool _useValidationLayer = true;
#else
		const bool _useValidationLayer = false;
#endif
	};
}