#pragma once
#include "dodopch.h"

#define GLM_FORCE_RADIANS
#define GLF_FORCE_DEPTH_ZERO_TO_ONE


#include "environment/Log.h"
#include "environment/Error.h"

namespace Dodo
{
	namespace Rendering
	{
		using namespace Dodo::Environment;

		class VKIntegration
		{
			struct QueueFamilies
			{
				uint32_t graphicsQueueFamilyIndices;
				uint32_t presentQueueFamilyIndices;
			};

			struct Queues
			{
				VkQueue	graphicsQueue;
				VkQueue	presentQueue;
			};

		public:
			VkResult CreateInstance();
			VkResult ChoosePhysicalDevice();
			VkResult CreateLogicalDevice(VkSurfaceKHR *_surface);

			uint32_t FindMemoryType(uint32_t _typeFilter, VkMemoryPropertyFlags _properties);

			VkPhysicalDeviceLimits GetPhysDevLimits();

			void Finalize();

			// getter/setter
			VkInstance				   const vulkanInstance()  const { return m_vkInstance; };
			VkPhysicalDevice		   const physicalDevice()  const { return m_vkPhysicalDevice; }
			VkDevice				   const device()		   const { return m_vkDevice; }
			VkSurfaceKHR			   const surface()		   const { return m_vkWindowSurface; }
			QueueFamilies			   const queueFamilies()   const { return m_queueFamilies; }
			Queues					   const queues()		   const { return m_queues; }
			VkPhysicalDeviceProperties const physDeviceProps() const { return m_vkDeviceProps; }
			VkPhysicalDeviceFeatures   const physDevFeatures() const { return m_vkDeviceFeatures; }


		private:
			bool	 IsDeviceSuitable(VkPhysicalDevice _device);
			bool	 CheckValidationLayerSupport();
			VkResult SetupValidationLayers();
			void	 QueryQueueFamilies(VkSurfaceKHR *_surface);
			std::vector<const char*> GetRequiredExtensions();

			VkResult CreateDebugReportCallbackEXT(
						VkInstance								  vulkanInstance, 
						const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, 
						const VkAllocationCallbacks				 *pAllocator, 
						VkDebugReportCallbackEXT				 *pDebugMessenger);

			static void DestroyDebugReportCallbackEXT(
						VkInstance					 vulkanInstance,
						VkDebugReportCallbackEXT	 pCallback,
						const VkAllocationCallbacks *pAllocator);

			static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
				VkDebugReportFlagsEXT	   flags,
				VkDebugReportObjectTypeEXT objType,
				uint64_t				   obj,
				size_t					   location,
				int32_t					   code,
				const char*				   layerPrefix,
				const char*				   msg,
				void*					   userData);

			std::vector<char const*> const validationLayers =
			{
#ifdef _DEBUG
				"VK_LAYER_LUNARG_standard_validation"
#endif
			};


			// Vulkan Handles (evtl. in ein struct verpacken für übersicht?)
			VkInstance				   m_vkInstance		  = VK_NULL_HANDLE;
			VkSurfaceKHR			   m_vkWindowSurface  = VK_NULL_HANDLE;
			VkPhysicalDevice		   m_vkPhysicalDevice = VK_NULL_HANDLE;
			VkDevice				   m_vkDevice		  = VK_NULL_HANDLE;
			VkDebugReportCallbackEXT   m_vkCallback		  = VK_NULL_HANDLE;
			VkPhysicalDeviceFeatures   m_vkDeviceFeatures = {};
			VkPhysicalDeviceProperties m_vkDeviceProps    = {};
			VkApplicationInfo		   m_vkAppInfo		  = {};
			VkInstanceCreateInfo	   m_vkInstanceInfo	  = {};

			std::vector<const char*> m_strExtensions;
			const std::vector<const char*> m_strDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			QueueFamilies			 m_queueFamilies = {};
			Queues					 m_queues = {};


		};
	}
}