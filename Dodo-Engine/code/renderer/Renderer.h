#pragma once
#include "common/VKIntegration.h"
#include "common/VKHelpers.h"
#include "environment/Window.h"
#include "Material.h"
#include "common/DodoTypes.h"

#include <algorithm>

namespace Dodo
{
	namespace Rendering
	{
		class CRenderer
		{
			struct SwapChainSupportDetails
			{
				VkSurfaceCapabilitiesKHR		capabilities;
				std::vector<VkSurfaceFormatKHR> formats;
				std::vector<VkPresentModeKHR>	presentModes;
			};

		public:
			CRenderer(std::vector<std::shared_ptr<Rendering::CMaterial>> _materials)
				: m_pMaterials(_materials)
			{

			}

			DodoError Initialize(std::shared_ptr<VKIntegration> _integration, std::shared_ptr<CWindow> _window);
			DodoError DrawFrame();

			DodoError Finalize();

			static void framebufferResizeCallback(GLFWwindow *_window, int _width, int _height) 
			{
				m_bFramebufferResized = true;
			}

		private:

			// Methods
			VkResult CreateSwapChain();
			VkResult CreateImageViews();
			VkResult CreateRenderPass();
			VkResult CreateDescriptorSetLayout();
			VkResult CreateGraphicsPipeline();
			VkResult CreateFramebuffers();
			VkResult CreateCommandPool();
			VkResult CreateCommandBuffers();
			VkResult CreateSyncObjects();

			VkResult CleanupSwapChain();
			VkResult RecreateSwapChain();

			SwapChainSupportDetails QuerySwapChainSupport();

			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats);
			VkPresentModeKHR   ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> _availablePresentModes);
			VkExtent2D		   ChooseSwapExtent(const VkSurfaceCapabilitiesKHR & _capabilities);

			struct SyncObjects
			{
				std::vector<VkSemaphore> imageAvailableSemaphores;
				std::vector<VkSemaphore> renderFinishedSemaphores;
				std::vector<VkFence>     inFlightFences;
			};

			// Variables
			VkSwapchainKHR				 m_vkSwapChain		       = VK_NULL_HANDLE;
			VkRenderPass				 m_vkRenderPass		       = VK_NULL_HANDLE;
			VkDescriptorSetLayout		 m_vkDescriptorSetLayout   = VK_NULL_HANDLE;
			VkPipelineLayout			 m_vkPipelineLayout	       = VK_NULL_HANDLE;
			VkPipeline					 m_vkGraphicsPipeline	   = VK_NULL_HANDLE;
			VkCommandPool				 m_vkCommandPool		   = VK_NULL_HANDLE;
			std::vector<VkImage>		 m_vkSwapChainImages	   = {};
			std::vector<VkImageView>	 m_vkSwapChainImageViews   = {};
			std::vector<VkFramebuffer>	 m_vkSwapChainFramebuffers = {};
			std::vector<VkCommandBuffer> m_vkCommandBuffers		   = {};
			VkFormat				     m_vkSwapChainImageFormat;
			VkExtent2D				     m_vkSwapChainExtent;
			SyncObjects				     m_sSyncObjects;

			std::shared_ptr<VKIntegration> m_pIntegration;
			std::shared_ptr<CWindow>       m_pWindow;
			std::vector<std::shared_ptr<Rendering::CMaterial>>       m_pMaterials;

			uint32_t	   m_iCurrentFrame      = 0;
			const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

			static bool m_bFramebufferResized;

		};
	}
}