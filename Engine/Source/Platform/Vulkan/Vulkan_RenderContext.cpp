#include "gepch.h"
#include "GlassEngine/Utilities/Logger.h"
#include "Vulkan_RenderContext.h"
#include "Vulkan_Types.h"
#include "Vulkan_DescriptorManager.h"
#include <exception>
#include <set>
#include <vulkan/vulkan_core.h>

#define FEATURE_MUST_BE_SUPPORTED(feature) if (bool(feature) != VK_TRUE) { GE_GRAPHICS_ERROR("{} must be supported", #feature); isSuitable = false; }
#define PROPERTIES_MUST_BE_GREATER_THAN(feature, value) if (bool(feature < value)) { GE_GRAPHICS_ERROR("{} must be at least {}", #feature, value); isSuitable = false; }
// TODO (dnm): better name
#define VK_SET_EXT_FUNC(name) name = (PFN_##name)vkGetInstanceProcAddr(_instance, #name);

// VK_EXT_device_memory_report
namespace ge::renderer {
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			GE_GRAPHICS_ERROR("validation layer: {}", pCallbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			GE_GRAPHICS_WARN("validation layer: {}", pCallbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			GE_GRAPHICS_INFO("validation layer: {}", pCallbackData->pMessage);
		}
		return VK_FALSE;
	}

	Vulkan_RenderContext::Vulkan_RenderContext(GLFWwindow* window) : _window(window) {}
	Vulkan_RenderContext::~Vulkan_RenderContext() {
		delete _allocator;
		_allocator = nullptr;

		_descriptorManager.reset();

		vkDestroyPipelineCache(_device, _pipelineCache, VK_ALLOCATOR_CALLBACKS);
		vkDestroyCommandPool(_device, _commandPool, VK_ALLOCATOR_CALLBACKS);

		vkDestroyDevice(_device, VK_ALLOCATOR_CALLBACKS);
		vkDestroySurfaceKHR(_instance, _surface, VK_ALLOCATOR_CALLBACKS);
		if (_debugMessenger) vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, VK_ALLOCATOR_CALLBACKS);
		vkDestroyInstance(_instance, VK_ALLOCATOR_CALLBACKS);
	}

	void Vulkan_RenderContext::Init()
	{
		GE_PROFILE_SCOPE("Create::RHI_Vulkan_Context");
		try {
			CreateVulkanAllocator();
			CreateInstance();
			CreateSurface();
			PickPhysicalDevice();
			CreateLogicalDevice();
			CreateDescriptorManager();
		}
		catch (const std::exception& e) {
			GE_GRAPHICS_ERROR("{}", e.what());
		}
	}

	void Vulkan_RenderContext::Wait() {
		// TODO (dnm): wait fence
		vkQueueWaitIdle(_graphicsQueue);
	}

	uint32_t Vulkan_RenderContext::IGetReadonlyImageHandle(Image& image, ImageSubresource subresource) {
		return _descriptorManager->RegisterReadonlyImage(static_cast<Vulkan_Image&>(image), subresource);
	}

	uint32_t Vulkan_RenderContext::IGetWritableImageHandle(Image& image, ImageSubresource subresource) {
		return _descriptorManager->RegisterWritableImage(static_cast<Vulkan_Image&>(image), subresource);
	}

	uint32_t Vulkan_RenderContext::IGetSamplerHandle(Sampler& sampler) {
		return _descriptorManager->RegisterSampler(static_cast<Vulkan_Sampler&>(sampler));
	}

	void Vulkan_RenderContext::CreateVulkanAllocator() {
		mem::Vulkan_AllocatorCallbacks::InitCallbacks();
		_allocator = new mem::Vulkan_Allocator();
	}

	void Vulkan_RenderContext::CreateInstance()
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = Engine::Get().GetApplicationSpecs().title.c_str();
		appInfo.pEngineName = Engine::Get().GetSpecs().name.c_str();
		appInfo.apiVersion = VK_API_VERSION_1_4;

		GEVector<const char*> instanceExtensions = GetRequiredInstanceExtensions();
		if (_useValidationLayer) {
			_layers.push_back("VK_LAYER_KHRONOS_validation");
			_layers.push_back("VK_LAYER_KHRONOS_synchronization2");
		}
		if (!CheckEnabledLayersSupport()) {
			throw std::runtime_error("Layer is enabled but not available!");
		}

		constexpr VkValidationFeatureEnableEXT enabledValidationFeatures[2] = {
			VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
			VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
		};

		VkValidationFeaturesEXT validationFeatures{};
		validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		validationFeatures.enabledValidationFeatureCount = 2;
		validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures;

		VkDebugUtilsMessengerCreateInfoEXT debugcreateInfo{};
		debugcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugcreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugcreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugcreateInfo.pfnUserCallback = debugCallback;
		debugcreateInfo.pUserData = nullptr;
		debugcreateInfo.pNext = &validationFeatures;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(_layers.size());
		createInfo.ppEnabledLayerNames = _layers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		createInfo.pNext = _useValidationLayer ? &debugcreateInfo : nullptr;

		auto result = vkCreateInstance(&createInfo, VK_ALLOCATOR_CALLBACKS, &_instance);

		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create vulkan instance, result: " + std::to_string(result));

		{
			VK_SET_EXT_FUNC(vkSetDebugUtilsObjectNameEXT);
			VK_SET_EXT_FUNC(vkCreateDebugUtilsMessengerEXT);
			VK_SET_EXT_FUNC(vkDestroyDebugUtilsMessengerEXT);
			VK_SET_EXT_FUNC(vkCmdBeginDebugUtilsLabelEXT);
			VK_SET_EXT_FUNC(vkCmdEndDebugUtilsLabelEXT);
		}

		if (_useValidationLayer) {
			result = vkCreateDebugUtilsMessengerEXT(_instance, &debugcreateInfo, VK_ALLOCATOR_CALLBACKS, &_debugMessenger);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to set up debug messenger!, result: " + std::to_string(result));
			}
		}
	}

	void Vulkan_RenderContext::PickPhysicalDevice() {
		uint32_t physicalDeviceCount;
		vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);
		GEVector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices.data());

		for (const auto& pdevice : physicalDevices) {
			if (IsPhysicalDeviceSuitable(pdevice)) {
				_physicalDevice = pdevice;
				if (_deviceFeatures.discrateGpu) break;
			}
		}

		if (!_physicalDevice)
			throw std::runtime_error("there is no supported gpu!");

		FindQueueFamilies();

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);
		GE_GRAPHICS_INFO("GPU Selected:");
		GE_GRAPHICS_INFO("GPU Name: {}", physicalDeviceProperties.deviceName);
		if (_deviceFeatures.discrateGpu) GE_GRAPHICS_INFO("GPU is discrete");
		if (_deviceFeatures.unifiedImageLayouts) GE_GRAPHICS_INFO("GPU supported VK_KHR_unified_image_layouts extension");
		if (_deviceFeatures.descriptorHeap) GE_GRAPHICS_INFO("GPU supported VK_EXT_descriptor_heap extension");
		if (_deviceFeatures.hostImageCopy) GE_GRAPHICS_INFO("GPU supported hostImageCopy feature");
		if (_deviceFeatures.partiallyBoundForSampler) GE_GRAPHICS_INFO("GPU supported partiallyBoundForSampler feature");
		if (_deviceFeatures.partiallyBoundForSampledImage) GE_GRAPHICS_INFO("GPU supported partiallyBoundForSampledImage feature");
		if (_deviceFeatures.partiallyBoundForStorageImage) GE_GRAPHICS_INFO("GPU supported partiallyBoundForStorageImage feature");
		if (_deviceFeatures.umaOrRebar) GE_GRAPHICS_INFO("GPU supported umaOrRebar feature");
	}

	void Vulkan_RenderContext::CreateSurface()
	{
		if (glfwCreateWindowSurface(_instance, _window, VK_ALLOCATOR_CALLBACKS, &_surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");
	}

	void Vulkan_RenderContext::CreateLogicalDevice() {
		_deviceExtensions = GetRequiredDeviceExtensions();
		_deviceExtensions.emplace_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

		void* pNext = nullptr;

		// TODO (dnm): change with pipeline robustness
#ifdef _DEBUG
		VkPhysicalDeviceRobustness2FeaturesEXT robustness2{};
		robustness2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		robustness2.nullDescriptor = true;
		robustness2.robustBufferAccess2 = true;
		robustness2.robustImageAccess2 = true;

		_deviceExtensions.emplace_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
		robustness2.pNext = pNext;
		pNext = &robustness2;
#endif

		VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeap{};
		descriptorHeap.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT;
		descriptorHeap.descriptorHeap = true;
		if (_deviceFeatures.descriptorHeap)
		{
			_deviceExtensions.emplace_back(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
			descriptorHeap.pNext = pNext;
			pNext = &descriptorHeap;
		}

		VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayouts{};
		unifiedImageLayouts.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR;
		unifiedImageLayouts.unifiedImageLayouts = true;
		if (_deviceFeatures.unifiedImageLayouts)
		{
			_deviceExtensions.emplace_back(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);
			unifiedImageLayouts.pNext = pNext;
			pNext = &unifiedImageLayouts;
		}

		VkPhysicalDeviceVulkan14Features features14{};
		features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
		features14.pNext = pNext;
		pNext = &features14;

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

		features14.hostImageCopy = _deviceFeatures.hostImageCopy;
		// TODO (dnm): enable for debug mode
		// features14.pipelineRobustness = true;
		features14.pushDescriptor = true;
		features14.maintenance5 = true;
		features13.synchronization2 = true;
		features13.dynamicRendering = true;
		features13.inlineUniformBlock = true;
		features12.descriptorBindingSampledImageUpdateAfterBind = true;
		features12.descriptorBindingStorageImageUpdateAfterBind = true;
		features12.descriptorBindingStorageBufferUpdateAfterBind = true;
		features12.descriptorBindingStorageImageUpdateAfterBind = true;
		features12.shaderSampledImageArrayNonUniformIndexing = true;
		features12.shaderStorageBufferArrayNonUniformIndexing = true;
		features12.shaderStorageImageArrayNonUniformIndexing = true;
		features12.shaderUniformBufferArrayNonUniformIndexing = true;
		features12.descriptorBindingUpdateUnusedWhilePending = true;
		features12.descriptorBindingVariableDescriptorCount = true;
		features12.descriptorBindingPartiallyBound = true;
		features12.descriptorIndexing = true;
		features12.runtimeDescriptorArray = true;
		features12.bufferDeviceAddress = true;
		features12.uniformBufferStandardLayout = true;
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
#ifdef _DEBUG
		features.features.robustBufferAccess = true;
#endif
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

	void Vulkan_RenderContext::CreateDescriptorManager() {
		if (_deviceFeatures.descriptorHeap && false) {
			_descriptorManager = std::make_unique<Vulkan_DescriptorManagerDefault>(*this);
		}
		else {
			_descriptorManager = std::make_unique<Vulkan_DescriptorManagerDefault>(*this);
		}
	}

	// TODO (dnm): log for unsupported features
	bool Vulkan_RenderContext::IsPhysicalDeviceSuitable(VkPhysicalDevice device) {
		bool isSuitable = true;
		_deviceFeatures = {};

		{
			uint32_t extCount{};
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
			GEVector<VkExtensionProperties> extensions(extCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, extensions.data());
	
			for (const auto& extension : extensions) {
				const std::string extensionName = extension.extensionName;
	
				if (extensionName == VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME) {
					_deviceFeatures.unifiedImageLayouts = true; // supportedExtensions.push_back(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);
				}
	
				if (extensionName == VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME) {
					_deviceFeatures.descriptorHeap = true; // supportedExtensions.push_back(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
				}
			}
		}

		// check features
		{
			void* pNext = nullptr;

			VkPhysicalDeviceVulkan14Features features14{};
			features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
			features14.pNext = pNext;
			pNext = &features14;

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

			vkGetPhysicalDeviceFeatures2(device, &features);

			_deviceFeatures.hostImageCopy = features14.hostImageCopy;
			FEATURE_MUST_BE_SUPPORTED(features14.pushDescriptor)
			FEATURE_MUST_BE_SUPPORTED(features14.maintenance5)
			FEATURE_MUST_BE_SUPPORTED(features13.synchronization2)
			FEATURE_MUST_BE_SUPPORTED(features13.dynamicRendering)
			FEATURE_MUST_BE_SUPPORTED(features13.inlineUniformBlock)
			FEATURE_MUST_BE_SUPPORTED(features12.descriptorBindingSampledImageUpdateAfterBind)
			FEATURE_MUST_BE_SUPPORTED(features12.descriptorBindingStorageImageUpdateAfterBind)
			FEATURE_MUST_BE_SUPPORTED(features12.descriptorBindingStorageBufferUpdateAfterBind)
			FEATURE_MUST_BE_SUPPORTED(features12.descriptorBindingUpdateUnusedWhilePending)
			FEATURE_MUST_BE_SUPPORTED(features12.descriptorBindingVariableDescriptorCount)
			FEATURE_MUST_BE_SUPPORTED(features12.shaderSampledImageArrayNonUniformIndexing)
			FEATURE_MUST_BE_SUPPORTED(features12.shaderStorageBufferArrayNonUniformIndexing)
			FEATURE_MUST_BE_SUPPORTED(features12.shaderStorageImageArrayNonUniformIndexing)
			FEATURE_MUST_BE_SUPPORTED(features12.shaderUniformBufferArrayNonUniformIndexing)
			FEATURE_MUST_BE_SUPPORTED(features12.descriptorIndexing)
			FEATURE_MUST_BE_SUPPORTED(features12.runtimeDescriptorArray)
			FEATURE_MUST_BE_SUPPORTED(features12.bufferDeviceAddress)
			FEATURE_MUST_BE_SUPPORTED(features12.hostQueryReset)
			FEATURE_MUST_BE_SUPPORTED(features12.uniformBufferStandardLayout)
			FEATURE_MUST_BE_SUPPORTED(features12.drawIndirectCount)
			FEATURE_MUST_BE_SUPPORTED(features12.vulkanMemoryModel)
			FEATURE_MUST_BE_SUPPORTED(features12.scalarBlockLayout)
			FEATURE_MUST_BE_SUPPORTED(features12.separateDepthStencilLayouts)
			FEATURE_MUST_BE_SUPPORTED(features11.shaderDrawParameters)
			FEATURE_MUST_BE_SUPPORTED(features11.uniformAndStorageBuffer16BitAccess)
			FEATURE_MUST_BE_SUPPORTED(features11.storageBuffer16BitAccess)
			FEATURE_MUST_BE_SUPPORTED(features.features.pipelineStatisticsQuery)
			FEATURE_MUST_BE_SUPPORTED(features.features.samplerAnisotropy)
			FEATURE_MUST_BE_SUPPORTED(features.features.tessellationShader)
			FEATURE_MUST_BE_SUPPORTED(features.features.geometryShader)
			FEATURE_MUST_BE_SUPPORTED(features.features.shaderInt16)
			FEATURE_MUST_BE_SUPPORTED(features.features.textureCompressionBC)
			FEATURE_MUST_BE_SUPPORTED(features.features.drawIndirectFirstInstance)
		}

		{
			void* pNext = nullptr;
			
			VkPhysicalDeviceVulkan14Properties properties14{};
			properties14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES;
			properties14.pNext = pNext;
			pNext = &properties14;

			VkPhysicalDeviceVulkan13Properties properties13{};
			properties13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
			properties13.pNext = pNext;
			pNext = &properties13;

			VkPhysicalDeviceVulkan12Properties properties12{};
			properties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
			properties12.pNext = pNext;
			pNext = &properties12;

			VkPhysicalDeviceVulkan11Properties properties11{};
			properties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
			properties11.pNext = pNext;
			pNext = &properties11;

			VkPhysicalDeviceProperties2 properties{};
			properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			properties.pNext = pNext;
			pNext = &properties;

			vkGetPhysicalDeviceProperties2(device, &properties);
	
			PROPERTIES_MUST_BE_GREATER_THAN(properties.properties.limits.maxPushConstantsSize, 256);

			_deviceFeatures.partiallyBoundForSampledImage = properties.properties.limits.maxPerStageDescriptorSampledImages > 200'000;
			_deviceFeatures.partiallyBoundForStorageImage = properties.properties.limits.maxPerStageDescriptorStorageImages > 10'000;
			_deviceFeatures.partiallyBoundForSampler = properties.properties.limits.maxPerStageDescriptorSamplers > 1'000;

			if (!_deviceFeatures.partiallyBoundForSampledImage)
				PROPERTIES_MUST_BE_GREATER_THAN(properties12.maxPerStageDescriptorUpdateAfterBindSampledImages, VULKAN_SAMPLED_IMAGE_COUNT);
			if (!_deviceFeatures.partiallyBoundForStorageImage)
				PROPERTIES_MUST_BE_GREATER_THAN(properties12.maxPerStageDescriptorUpdateAfterBindStorageImages, VULKAN_STORAGE_IMAGE_COUNT);
			if (!_deviceFeatures.partiallyBoundForSampler)
				PROPERTIES_MUST_BE_GREATER_THAN(properties12.maxPerStageDescriptorUpdateAfterBindSamplers, VULKAN_SAMPLER_COUNT);

			FEATURE_MUST_BE_SUPPORTED(properties11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT);
			FEATURE_MUST_BE_SUPPORTED(properties11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT);

			FEATURE_MUST_BE_SUPPORTED(properties11.subgroupSupportedStages & VK_SHADER_STAGE_FRAGMENT_BIT);
			FEATURE_MUST_BE_SUPPORTED(properties11.subgroupSupportedStages & VK_SHADER_STAGE_COMPUTE_BIT);

			_deviceFeatures.discrateGpu = properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			_deviceFeatures.umaOrRebar = !_deviceFeatures.discrateGpu;

			VkPhysicalDeviceMemoryProperties2 memoryProperties{};
			memoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;

			vkGetPhysicalDeviceMemoryProperties2(device, &memoryProperties);

			for (const auto memType : memoryProperties.memoryProperties.memoryTypes) {
				const auto heap = memoryProperties.memoryProperties.memoryHeaps[memType.heapIndex];
				if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT && heap.size > ((1 << 20) * 512)) {
					_deviceFeatures.umaOrRebar |= 
						memType.propertyFlags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				}
			}

			const auto supportedSamples = properties.properties.limits.framebufferColorSampleCounts;

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

		return isSuitable;
	}

	void Vulkan_RenderContext::ISetUniformBuffer(ge::mem::Ref<Buffer>& buffer, uint32_t binding) {
		_descriptorManager->SetUniformBuffer(*buffer.Cast<Vulkan_Buffer>(), binding);
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
			VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME,
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	}
}
