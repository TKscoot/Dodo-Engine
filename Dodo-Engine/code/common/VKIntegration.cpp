#include "VKIntegration.h"

using namespace Dodo;
using namespace Dodo::Environment;

VkResult Dodo::Rendering::VKIntegration::CreateInstance()
{
	// Creating Vulkan vulkanInstance
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Dodo Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "Dodo Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo instanceInfo= {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;

	auto extensions = GetRequiredExtensions();

	instanceInfo.enabledExtensionCount = extensions.size();
	instanceInfo.ppEnabledExtensionNames = extensions.data();
	instanceInfo.ppEnabledLayerNames = validationLayers.data();	//evtl noch einbauen
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());



	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_vkInstance);
	Environment::CError::CheckError<VkResult>(result);

	// Configuring and creating callbacks for validation layer
	result = SetupValidationLayers();
	Environment::CError::CheckError<VkResult>(result);

	// Picking Physical Device
	result = ChoosePhysicalDevice();
	Environment::CError::CheckError<VkResult>(result);


	return result;
}

VkResult Dodo::Rendering::VKIntegration::ChoosePhysicalDevice()
{
	uint32_t deviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices (deviceCount);
	result = vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, physicalDevices.data());
	Environment::CLog::Message(std::to_string(deviceCount) + " Device(s) found!");

	std::multimap<int, VkPhysicalDevice> devices;
	for (auto physicalDevice : physicalDevices)
	{
		if (IsDeviceSuitable(physicalDevice))
		{
			int score = 0;
			VkPhysicalDeviceProperties properties = {};
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);
			switch (properties.deviceType) // höchst unzufrieden. muss mir was besseres einfallen lassen.
			{
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				score += 1000;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score += 100;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				score += 10;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				score += 1;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
				score += 0;
				break;
			default:
				break;
			}
			devices.insert(std::make_pair(score, physicalDevice));
		}

	}

	m_vkPhysicalDevice = devices.rbegin()->second;

	return result;
}

bool Dodo::Rendering::VKIntegration::IsDeviceSuitable(VkPhysicalDevice _device)
{
	vkGetPhysicalDeviceFeatures(_device, &m_vkDeviceFeatures);
	bool const supportsGeometryShaders		= m_vkDeviceFeatures.geometryShader;
	bool const supportsTesselationShaders	= m_vkDeviceFeatures.tessellationShader;
	bool const supportsLogicalBlendOps		= m_vkDeviceFeatures.logicOp;
	bool const supportsNonSolidDrawing		= m_vkDeviceFeatures.fillModeNonSolid;
	bool const supportsAnisotropicSampler	= m_vkDeviceFeatures.samplerAnisotropy;
	bool const supportsBlockCompressedFmt	= m_vkDeviceFeatures.textureCompressionBC;

	bool const allFeaturesSupported = (
				supportsGeometryShaders    &&
				supportsTesselationShaders &&
				supportsLogicalBlendOps    &&
				supportsNonSolidDrawing    &&
				supportsAnisotropicSampler &&
				supportsBlockCompressedFmt);

	return allFeaturesSupported;
}

VkResult Dodo::Rendering::VKIntegration::CreateLogicalDevice(VkSurfaceKHR *_surface)
{
	m_vkWindowSurface = *_surface;
	QueryQueueFamilies(_surface);
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { m_queueFamilies.graphicsQueueFamilyIndices, m_queueFamilies.presentQueueFamilyIndices };

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	for (int queueFamily : uniqueQueueFamilies)
	{
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = m_queueFamilies.graphicsQueueFamilyIndices;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo;
	createInfo.sType				   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext				   = 0;
	createInfo.flags				   = 0;
	createInfo.queueCreateInfoCount    = queueCreateInfos.size();
	createInfo.pQueueCreateInfos	   = queueCreateInfos.data();
	createInfo.enabledLayerCount       = validationLayers.size();
	createInfo.ppEnabledLayerNames	   = validationLayers.data();
	createInfo.enabledExtensionCount   = m_strDeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = m_strDeviceExtensions.data();
	createInfo.pEnabledFeatures		   = &m_vkDeviceFeatures;

	VkResult result = vkCreateDevice(m_vkPhysicalDevice, &createInfo, nullptr, &m_vkDevice);
	CError::CheckError<VkResult>(result);

	vkGetDeviceQueue(m_vkDevice, m_queueFamilies.graphicsQueueFamilyIndices, 0, &m_queues.graphicsQueue);
	vkGetDeviceQueue(m_vkDevice, queueCreateInfo.queueFamilyIndex, 0, &m_queues.presentQueue);

	return result;
}

VkResult Dodo::Rendering::VKIntegration::SetupValidationLayers()
{
	VkResult result;

	uint32_t const reportFlags =
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

	if (!CheckValidationLayerSupport())
	{
		return VK_ERROR_LAYER_NOT_PRESENT; // evtl falscher return code
	}

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = reportFlags;
	createInfo.pfnCallback = debugCallback;

	result = CreateDebugReportCallbackEXT(m_vkInstance, &createInfo, nullptr, &m_vkCallback);
	CError::CheckError<VkResult>(result);

	return result;
}

bool Dodo::Rendering::VKIntegration::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

void Dodo::Rendering::VKIntegration::QueryQueueFamilies(VkSurfaceKHR *_surface)
{
	uint32_t queueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, nullptr);

	VkBool32 presentSupport = false;

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_queueFamilies.graphicsQueueFamilyIndices = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, *_surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			m_queueFamilies.presentQueueFamilyIndices = i;
		}

		i++;
	}


}

std::vector<const char*> Dodo::Rendering::VKIntegration::GetRequiredExtensions()
{

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
	{
		
		m_strExtensions.push_back(glfwExtensions[i]);
	}

	m_strExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	return m_strExtensions;
}

VkResult Dodo::Rendering::VKIntegration::CreateDebugReportCallbackEXT(
	VkInstance vulkanInstance, 
	const VkDebugReportCallbackCreateInfoEXT * pCreateInfo, 
	const VkAllocationCallbacks * pAllocator, 
	VkDebugReportCallbackEXT * pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanInstance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(vulkanInstance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void Dodo::Rendering::VKIntegration::DestroyDebugReportCallbackEXT(
	VkInstance vulkanInstance, 
	VkDebugReportCallbackEXT pCallback, 
	const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanInstance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(vulkanInstance, pCallback, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Dodo::Rendering::VKIntegration::debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	switch (flags)
	{
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
		CLog::Message("validation layer: " + std::string(msg), code);
		break;
	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
		CLog::Warning("validation layer: " + std::string(msg), code);
		break;
	case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		CLog::Error("validation layer: " + std::string(msg), code);
		break;
	case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		CLog::Message("validation layer: " + std::string(msg), code);
		break;
	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
		CLog::Warning("validation layer: " + std::string(msg), code);
		break;

	default:
		break;
	}

	return VK_FALSE;
}
