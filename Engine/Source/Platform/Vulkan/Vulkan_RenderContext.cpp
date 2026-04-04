#include "gepch.h"
#include "Vulkan_RenderContext.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <exception>
#include <set>

namespace ge::renderer {
#define VULKAN_ALLOCATOR_CALLBACKS &mem::Vulkan_AllocatorCallbacks::GetCallbacks()
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			GE_GRAPCHICS_ERROR("validation layer: {}", pCallbackData->pMessage);
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

	Vulkan_RenderContext::Vulkan_RenderContext(GLFWwindow* window) : _window(window) { }
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
			GE_GRAPCHICS_ERROR(e.what());
		}
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
		if (_useValidationLayer) { _layers.push_back("VK_LAYER_KHRONOS_validation"); }
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
		if(vkCreateInstance(&createInfo, VULKAN_ALLOCATOR_CALLBACKS, &_instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create vulkan instance");
		GE_GRAPCHICS_INFO("Vulkan instance created");
	}

	void Vulkan_RenderContext::CreateDebugMessenger() {
		if (!_useValidationLayer) return;
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
		if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, VULKAN_ALLOCATOR_CALLBACKS, &_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
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
				break;
			}
		}
		vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);
		GE_GRAPCHICS_INFO("GPU Selected: ");
		GE_GRAPCHICS_INFO("	GPU Name: {}", _physicalDeviceProperties.deviceName);
	}

	void Vulkan_RenderContext::CreateSurface()
	{
		if (glfwCreateWindowSurface(_instance, _window, VULKAN_ALLOCATOR_CALLBACKS, &_surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");
	}

	void Vulkan_RenderContext::CreateLogicalDevice() {
		QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);
		float queuePriority = 1.0f;
		GEVector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqeQueueFamilies = { indices.graphicsIndex.value(), indices.presentIndex.value() };
		for (uint32_t queueFamily : uniqeQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledLayerCount = static_cast<uint32_t>(_layers.size());
		createInfo.ppEnabledLayerNames = _layers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
		if (vkCreateDevice(_physicalDevice, &createInfo, VULKAN_ALLOCATOR_CALLBACKS, &_device) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");

		vkGetDeviceQueue(_device, indices.graphicsIndex.value(), 0, &_graphicsQueue);
		vkGetDeviceQueue(_device, indices.presentIndex.value(), 0, &_presentQueue);

		_allocator->CreateAllocator(_instance, _physicalDevice, _device);
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

	bool Vulkan_RenderContext::IsPhysicalDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = FindQueueFamilies(device);
		return indices.IsValid();
	}

	QueueFamilyIndices Vulkan_RenderContext::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;
		uint32_t queueFamiliesCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);
		GEVector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamiliesCount; i++) {
			const auto& queueFamily = queueFamilies[i];
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphicsIndex = i;

			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
			if (presentSupport)
				indices.presentIndex = i;

			if (indices.IsValid())
				break;
		}
		return indices;
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
}