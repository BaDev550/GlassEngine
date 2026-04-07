#include "gepch.h"
#include "Vulkan_RenderContext.h"
#include "Vulkan_Types.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <exception>
#include <set>

namespace ge::renderer {
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			GE_GRAPHICS_ERROR("validation layer: {}", pCallbackData->pMessage);
		return VK_FALSE;
	}
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	Vulkan_RenderContext::Vulkan_RenderContext(GLFWwindow* window) : _window(window) {}
	Vulkan_RenderContext::~Vulkan_RenderContext() {
		delete _allocator;
		_allocator = nullptr;
	}

	void Vulkan_RenderContext::Init()
	{
		try {
			CreateVulkanAllocator();
			CreateInstance();
			CreateSurface();
			PickPhysicalDevice();
			CreateLogicalDevice();
		}
		catch (const std::exception& e) {
			GE_GRAPHICS_ERROR(e.what());
		}
	}

	void Vulkan_RenderContext::Wait() {
		vkQueueWaitIdle(_graphicsQueue);
	}

	void Vulkan_RenderContext::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImage& image, VmaAllocation& alloc)
	{
		VkImageCreateInfo createInfo{};
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.extent.width = width;
		createInfo.extent.height = height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = 1;
		createInfo.tiling = tiling;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.format = format;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		// _allocator->AllocateImage(image, alloc, createInfo, memoryUsage);
	}

	void Vulkan_RenderContext::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{

	}

	void Vulkan_RenderContext::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
	}

	void Vulkan_RenderContext::CreateVulkanAllocator() {
		mem::Vulkan_AllocatorCallbacks::InitCallbacks();
		_allocator = new mem::Vulkan_Allocator();
	}

	void Vulkan_RenderContext::CreateInstance()
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = Application::Get()->GetSpecs().title.c_str();
		appInfo.pEngineName = "GlassEngine";
		appInfo.apiVersion = VK_API_VERSION_1_3;

		GEVector<const char*> instanceExtensions = GetRequiredInstanceExtensions();
		if (_useValidationLayer) {
			_layers.push_back("VK_LAYER_KHRONOS_validation");
			_layers.push_back("VK_LAYER_KHRONOS_synchronization2");
		}
		if (!CheckEnabledLayersSupport()) {
			throw std::runtime_error("Layer is enabled but not available!");
		}

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(_layers.size());
		createInfo.ppEnabledLayerNames = _layers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		if (vkCreateInstance(&createInfo, VK_ALLOCATOR_CALLBACKS, &_instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create vulkan instance");
		GE_GRAPHICS_INFO("Vulkan instance created");
	}

	void Vulkan_RenderContext::CreateDebugMessenger() {
		if (!_useValidationLayer) return;

		constexpr VkValidationFeatureEnableEXT validation_features[2] = {
			VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
			VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
		};

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
		createInfo.pNext = validation_features;
		if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, VK_ALLOCATOR_CALLBACKS, &_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	// TODO (dnm): select discrate gpu
	void Vulkan_RenderContext::PickPhysicalDevice() {
		uint32_t physicalDeviceCount;
		vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);
		GEVector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices.data());

		for (const auto& pdevice : physicalDevices) {
			if (IsPhysicalDeviceSuitable(pdevice)) {
				_physicalDevice = pdevice;
				break;
			}
		}

		FindQueueFamilies();

		uint32_t extCount{};
		vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr);
		GEVector<VkExtensionProperties> extensions(extCount);
		vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, extensions.data());

		GEVector<GEString> supportedExtensions{};
		for (const auto& extension : extensions) {
			const std::string extensionName = extension.extensionName;

			if (extensionName == VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) {
				_deviceFeatures.memoryBudget = true; supportedExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
			}

			if (extensionName == VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME) {
				_deviceFeatures.hostImageCopy = true; supportedExtensions.push_back(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME);
			}

			if (extensionName == VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME) {
				_deviceFeatures.unifiedImageLayouts = true; supportedExtensions.push_back(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);
			}

			if (extensionName == VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME) {
				_deviceFeatures.shaderStencilExport = true; supportedExtensions.push_back(VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME);
			}

			if (extensionName == VK_KHR_MAINTENANCE_5_EXTENSION_NAME) {
				_deviceFeatures.maintenance5 = true; supportedExtensions.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
			}
		}

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);
		GE_GRAPHICS_INFO("GPU Selected: ");
		GE_GRAPHICS_INFO("GPU Name: {}", physicalDeviceProperties.deviceName);
		for (const auto ext : supportedExtensions)
			GE_GRAPHICS_INFO("GPU supported optional extensions: {}", ext);

		physicalDeviceProperties.limits.framebufferColorSampleCounts;

		const auto supportedSamples = physicalDeviceProperties.limits.framebufferColorSampleCounts;

		_sampleMap[int(ImageSampleCount::e1)] = VK_SAMPLE_COUNT_1_BIT;
		_sampleMap[int(ImageSampleCount::e2)] = VK_SAMPLE_COUNT_1_BIT;
		_sampleMap[int(ImageSampleCount::e4)] = VK_SAMPLE_COUNT_1_BIT;
		_sampleMap[int(ImageSampleCount::e8)] = VK_SAMPLE_COUNT_1_BIT;

		if (VK_SAMPLE_COUNT_2_BIT & supportedSamples) {
			_sampleMap[int(ImageSampleCount::e2)] = VK_SAMPLE_COUNT_2_BIT;
		}
		else if (VK_SAMPLE_COUNT_4_BIT & supportedSamples) {
			_sampleMap[int(ImageSampleCount::e2)] = VK_SAMPLE_COUNT_4_BIT;
		}

		if (VK_SAMPLE_COUNT_4_BIT & supportedSamples) {
			_sampleMap[int(ImageSampleCount::e4)] = VK_SAMPLE_COUNT_4_BIT;
		}
		else if (VK_SAMPLE_COUNT_8_BIT & supportedSamples) {
			_sampleMap[int(ImageSampleCount::e4)] = VK_SAMPLE_COUNT_8_BIT;
		}
		else if (VK_SAMPLE_COUNT_2_BIT & supportedSamples) {
			_sampleMap[int(ImageSampleCount::e4)] = VK_SAMPLE_COUNT_2_BIT;
		}

		if (VK_SAMPLE_COUNT_8_BIT & supportedSamples) {
			_sampleMap[int(ImageSampleCount::e8)] = VK_SAMPLE_COUNT_8_BIT;
		}
		else if (VK_SAMPLE_COUNT_4_BIT & supportedSamples) {
			_sampleMap[int(ImageSampleCount::e8)] = VK_SAMPLE_COUNT_4_BIT;
		}
		else if (VK_SAMPLE_COUNT_2_BIT & supportedSamples) {
			_sampleMap[int(ImageSampleCount::e8)] = VK_SAMPLE_COUNT_2_BIT;
		}
	}

	void Vulkan_RenderContext::CreateSurface()
	{
		if (glfwCreateWindowSurface(_instance, _window, VK_ALLOCATOR_CALLBACKS, &_surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");
	}

	void Vulkan_RenderContext::CreateLogicalDevice() {
		void* pNext = nullptr;

		VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayouts{};
		unifiedImageLayouts.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR;
		unifiedImageLayouts.unifiedImageLayouts = true;
		if (_deviceFeatures.unifiedImageLayouts)
		{
			unifiedImageLayouts.pNext = pNext;
			pNext = &unifiedImageLayouts;
		}

		VkPhysicalDeviceHostImageCopyFeaturesEXT imageCopyFeatures{};
		imageCopyFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT;
		imageCopyFeatures.hostImageCopy = true;
		if (_deviceFeatures.hostImageCopy)
		{
			imageCopyFeatures.pNext = pNext;
			pNext = &imageCopyFeatures;
		}

		VkPhysicalDeviceVulkan13Features features13{};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.pNext = pNext;
		pNext = &features13;

		VkPhysicalDeviceVulkan12Features features12{};
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.pNext = pNext;
		pNext = &features12;

		VkPhysicalDeviceVulkan11Features features11{};
		features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		features11.pNext = pNext;
		pNext = &features11;

		VkPhysicalDeviceFeatures2 features{};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = pNext;
		pNext = &features;

		features13.synchronization2 = true;
		features13.dynamicRendering = true;
		features13.inlineUniformBlock = true;
		features12.descriptorBindingSampledImageUpdateAfterBind = true;
		features12.descriptorBindingStorageImageUpdateAfterBind = true;
		features12.descriptorBindingStorageBufferUpdateAfterBind = true;
		features12.descriptorBindingVariableDescriptorCount = true;
		features12.shaderSampledImageArrayNonUniformIndexing = true;
		features12.shaderStorageBufferArrayNonUniformIndexing = true;
		features12.shaderStorageImageArrayNonUniformIndexing = true;
		features12.shaderUniformBufferArrayNonUniformIndexing = true;
		features12.descriptorIndexing = true;
		features12.runtimeDescriptorArray = true;
		// if bufferDeviceAddress enabled bufferDeviceAddressCaptureReplay required for renderdoc, amd pre rdna gpu not support bufferDeviceAddressCaptureReplay
		// features12.bufferDeviceAddress = true;
		features12.hostQueryReset = true;
		features12.drawIndirectCount = true;
		features12.vulkanMemoryModel = true;
		features12.scalarBlockLayout = true;
		features12.separateDepthStencilLayouts = true;
		features11.shaderDrawParameters = true;
		features11.uniformAndStorageBuffer16BitAccess = true;
		features11.storageBuffer16BitAccess = true;
		features.features.pipelineStatisticsQuery = true;
		features.features.samplerAnisotropy = true;
		features.features.tessellationShader = true;
		features.features.geometryShader = true;
		features.features.shaderInt16 = true;
		features.features.textureCompressionBC = true;
		features.features.drawIndirectFirstInstance = true;

		constexpr float queuePriority = 1.0f;

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(_layers.size());
		createInfo.ppEnabledLayerNames = _layers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
		createInfo.pNext = &features;

		if (vkCreateDevice(_physicalDevice, &createInfo, VK_ALLOCATOR_CALLBACKS, &_device) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");

		vkGetDeviceQueue(_device, _graphicsQueueFamilyIndex, 0, &_graphicsQueue);

		_allocator->CreateAllocator(_instance, _physicalDevice, _device);
	}

	void Vulkan_RenderContext::CreatePipelineCache() {
		// TODO (dnm) : get pipeline cache from cache file
		// TODO (dnm) : write cache to file
		VkPipelineCacheCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vkCreatePipelineCache(_device, &createInfo, VK_ALLOCATOR_CALLBACKS, &_pipelineCache);
	}

	bool Vulkan_RenderContext::CheckEnabledLayersSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		GEVector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (const char* layerName : _layers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}
		return true;
	}

	// TODO (dnm): log for unsupported features
	bool Vulkan_RenderContext::IsPhysicalDeviceSuitable(VkPhysicalDevice device) {
		void* pNext = nullptr;

		VkPhysicalDeviceVulkan13Features features13;
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.pNext = pNext;
		pNext = &features13;

		VkPhysicalDeviceVulkan12Features features12;
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.pNext = pNext;
		pNext = &features12;

		VkPhysicalDeviceVulkan11Features features11;
		features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		features11.pNext = pNext;
		pNext = &features11;

		VkPhysicalDeviceFeatures2 features;
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = pNext;
		pNext = &features;

		vkGetPhysicalDeviceFeatures2(device, &features);

		if (features13.synchronization2 != VK_TRUE) return false;
		if (features13.dynamicRendering != VK_TRUE) return false;
		if (features13.inlineUniformBlock != VK_TRUE) return false;
		if (features12.descriptorBindingSampledImageUpdateAfterBind != VK_TRUE) return false;
		if (features12.descriptorBindingStorageImageUpdateAfterBind != VK_TRUE) return false;
		if (features12.descriptorBindingStorageBufferUpdateAfterBind != VK_TRUE) return false;
		if (features12.descriptorBindingVariableDescriptorCount != VK_TRUE) return false;
		if (features12.shaderSampledImageArrayNonUniformIndexing != VK_TRUE) return false;
		if (features12.shaderStorageBufferArrayNonUniformIndexing != VK_TRUE) return false;
		if (features12.shaderStorageImageArrayNonUniformIndexing != VK_TRUE) return false;
		if (features12.shaderUniformBufferArrayNonUniformIndexing != VK_TRUE) return false;
		if (features12.descriptorIndexing != VK_TRUE) return false;
		if (features12.runtimeDescriptorArray != VK_TRUE) return false;
		// if bufferDeviceAddress enabled bufferDeviceAddressCaptureReplay required for renderdoc, amd pre rdna gpu not support bufferDeviceAddressCaptureReplay
		// if (features12.bufferDeviceAddress != VK_TRUE) return false;
		if (features12.hostQueryReset != VK_TRUE) return false;
		if (features12.drawIndirectCount != VK_TRUE) return false;
		if (features12.vulkanMemoryModel != VK_TRUE) return false;
		if (features12.scalarBlockLayout != VK_TRUE) return false;
		if (features12.separateDepthStencilLayouts != VK_TRUE) return false;
		if (features11.shaderDrawParameters != VK_TRUE) return false;
		if (features11.uniformAndStorageBuffer16BitAccess != VK_TRUE) return false;
		if (features11.storageBuffer16BitAccess != VK_TRUE) return false;
		if (features.features.pipelineStatisticsQuery != VK_TRUE) return false;
		if (features.features.samplerAnisotropy != VK_TRUE) return false;
		if (features.features.tessellationShader != VK_TRUE) return false;
		if (features.features.geometryShader != VK_TRUE) return false;
		if (features.features.shaderInt16 != VK_TRUE) return false;
		if (features.features.textureCompressionBC != VK_TRUE) return false;
		if (features.features.drawIndirectFirstInstance != VK_TRUE) return false;

		return true;
	}

	void Vulkan_RenderContext::FindQueueFamilies()
	{
		uint32_t queueFamiliesCount;
		vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamiliesCount, nullptr);
		GEVector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamiliesCount, queueFamilies.data());

		// TODO (dnm): source
		// all graphics queues support presentation
		for (uint32_t i = 0; i < queueFamiliesCount; i++) {
			const auto& queueFamily = queueFamilies[i];
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				_graphicsQueueFamilyIndex = i;
				break;
			}
		}
	}

	GEVector<const char*> Vulkan_RenderContext::GetRequiredInstanceExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		GEVector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (_useValidationLayer) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	GEVector<const char*> Vulkan_RenderContext::GetRequiredDeviceExtensions() {
		return {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
		};
	}

	// TODO: finish this
	GEVector<const char*> Vulkan_RenderContext::GetSupportedOptionalDeviceExtensions() {
		GEVector<const char*> out;

		const GEVector<std::string> optExts{
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
			VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
			VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME,
			VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME,
			VK_KHR_MAINTENANCE_5_EXTENSION_NAME
		};

		uint32_t extCount{};
		vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr);
		GEVector<VkExtensionProperties> extensions(extCount);
		vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, extensions.data());

		for (const auto& extension : extensions) {
			const std::string extensionName = extension.extensionName;

			for (const auto& optExtName : optExts)
			{
				if (extensionName == optExtName)
				{
					out.push_back(extension.extensionName);
				}
			}
		}

		return out;
	}
}
