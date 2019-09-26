#pragma once
#include "dodopch.h"
#include "common/VKIntegration.h"
#include "common/VKHelpers.h"
#include "environment/Window.h"
#include "components/Material.h"
#include "common/DodoTypes.h"
#include "entity/EntityHandler.h"
#include "entity/Camera.h"
#include "GUI.h"

namespace Dodo
{
	namespace Rendering
	{
		using namespace Dodo::Components;

		class CRenderer
		{
			struct SwapChainSupportDetails
			{
				VkSurfaceCapabilitiesKHR		capabilities;
				std::vector<VkSurfaceFormatKHR> formats;
				std::vector<VkPresentModeKHR>	presentModes;
			};

		public:
			CRenderer(std::vector<std::shared_ptr<Components::CMesh>> _meshes, 
				std::vector<std::shared_ptr<Components::CMaterial>> _materials, 
				std::shared_ptr<Entity::CCamera> _camera,
				std::vector<std::shared_ptr<Entity::CEntity>> _entities)
				: //m_pMeshes(_meshes),
				//m_pMaterials(_materials),
				m_pCamera(_camera),
				m_pEntities(_entities)
			{
				m_pMaterials.clear();
				m_pMeshes.clear();
				m_pTransforms.clear();
				for (auto& ent : Entity::CEntityHandler::GetEntities())
				{
					std::shared_ptr<CMaterial> mat = std::shared_ptr<CMaterial>{ ent->GetComponent<CMaterial>() };
					if (mat != nullptr)
					{
						m_pMaterials.push_back(mat);
					}

					std::shared_ptr<CMesh> mesh = std::shared_ptr<CMesh>{ ent->GetComponent<CMesh>() };
					if (mesh != nullptr)
					{
						m_pMeshes.push_back(mesh);
					}

					std::shared_ptr<CTransform> transform = std::shared_ptr<CTransform>{ ent->GetComponent<CTransform>() };
					if (transform != nullptr)
					{
						m_pTransforms.push_back(transform);
					}
				}
				
			}

			DodoError Initialize(std::shared_ptr<VKIntegration> _integration, std::shared_ptr<CWindow> _window);
			DodoError DrawFrame(double deltaTime);

			DodoError Finalize();

			static void framebufferResizeCallback(GLFWwindow *_window, int _width, int _height) 
			{
				m_bFramebufferResized = true;
			}

			VkResult CreateBuffer(
				VkDeviceSize _size,
				VkBufferUsageFlags _usage,
				VkMemoryPropertyFlags _properties,
				VkBuffer &_buffer,
				VkDeviceMemory &_bufferMemory);

			// Getter & Setter
			VkRenderPass renderPass() { return m_vkRenderPass; }
			double const deltaTime() const { return m_dDeltaTime; }
			VkExtent2D const swapExtent() const { return m_vkSwapChainExtent; }
			void DrawGui(bool enabled) { m_bDrawGui = enabled; }
			void ToggleDrawGui() { m_bDrawGui = !m_bDrawGui; }

			VkResult UpdateCommandBuffers();


		private:

			// Methods
			VkResult CreateSwapChain();
			VkResult CreateImageViews();
			VkResult CreateRenderPass();
			VkResult CreateDescriptorSetLayout();
			VkResult CreateGraphicsPipeline();
			VkResult CreateFramebuffers();
			VkResult CreateCommandPool();
			VkResult CreateDepthResources();
			VkResult CreateTextureImages();
			VkResult CreateTextureImageViews();
			VkResult CreateTextureSampler();
			VkResult CreateVertexBuffers();
			VkResult CreateIndexBuffers();
			VkResult CreateUniformBuffers();
			VkResult CreateCommandBuffers();
			VkResult CreateSyncObjects();
			VkResult CreateDescriptorPool();
			VkResult CreateDescriptorSets();

			
// Image stuff
			VkResult CreateImage(
				uint32_t width, 
				uint32_t height, 
				VkFormat format, 
				VkImageTiling tiling,
				VkImageUsageFlags usage, 
				VkMemoryPropertyFlags properties, 
				VkImage& image, 
				VkDeviceMemory& imageMemory);

			VkResult TransitionImageLayout(
				VkImage image,
				VkFormat format,
				VkImageLayout oldLayout,
				VkImageLayout newLayout);

			VkResult CopyBufferToImage(
				VkBuffer _buffer, 
				VkImage _image, 
				uint32_t _width, 
				uint32_t _height);

			VkImageView CreateImageView(
				VkImage image, 
				VkFormat format, 
				VkImageAspectFlags aspectFlags);

			VkResult CreateTextureImage(CMaterial::Texture &_texture);

// Image stuff end

			VkResult CopyBuffer(
				VkBuffer _srcBuffer, 
				VkBuffer _dstBuffer, 
				VkDeviceSize _size);


			VkCommandBuffer BeginSingleTimeCommands();
			void EndSingleTimeCommands(VkCommandBuffer _buf);

			VkResult UpdateUniformBuffer();

			VkResult CleanupSwapChain();
			VkResult RecreateSwapChain();

			SwapChainSupportDetails QuerySwapChainSupport();

			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats);
			VkPresentModeKHR   ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> _availablePresentModes);
			VkExtent2D		   ChooseSwapExtent(const VkSurfaceCapabilitiesKHR & _capabilities);
			VkFormat		   FindSupportedFormat(
									const std::vector<VkFormat>& _candidates, 
									VkImageTiling _tiling, 
									VkFormatFeatureFlags _features);
			VkFormat FindDepthFormat()
			{
				return FindSupportedFormat(
					{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
					VK_IMAGE_TILING_OPTIMAL,
					VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
				);
			}
			bool HasStencilComponent(VkFormat _format)
			{
				return _format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT;
			}


			struct SyncObjects
			{
				std::vector<VkSemaphore> imageAvailableSemaphores;
				std::vector<VkSemaphore> renderFinishedSemaphores;
				std::vector<VkFence>     inFlightFences;
			};


			// Variables
			VkSwapchainKHR					   m_vkSwapChain		     = VK_NULL_HANDLE;
			VkRenderPass					   m_vkRenderPass		     = VK_NULL_HANDLE;
			VkDescriptorSetLayout			   m_vkDescriptorSetLayout	 = VK_NULL_HANDLE;
			VkPipelineLayout				   m_vkPipelineLayout	     = VK_NULL_HANDLE;
			VkPipeline						   m_vkGraphicsPipeline	     = VK_NULL_HANDLE;
			VkCommandPool					   m_vkCommandPool			 = VK_NULL_HANDLE;
			VkDescriptorPool				   m_vkDescriptorPool	     = VK_NULL_HANDLE;
			VkImage							   m_vkDepthImage			 = VK_NULL_HANDLE;
			VkDeviceMemory					   m_vkDepthImageMemory		 = VK_NULL_HANDLE;
			VkImageView						   m_vkDepthImageView		 = VK_NULL_HANDLE;
			std::vector<VkImage>			   m_vkSwapChainImages	     = {};
			std::vector<VkImageView>		   m_vkSwapChainImageViews   = {};
			std::vector<VkFramebuffer>		   m_vkSwapChainFramebuffers = {};
			std::vector<VkCommandBuffer>	   m_vkCommandBuffers		 = {};

			std::vector<VkDescriptorSet>       m_vkDescriptorSets		 = {};
			std::vector<CMesh::DataBuffer>	   m_matDataBuffers			 = {};
			std::vector<VkBuffer>			   m_vkUniformBuffers		 = {};
			std::vector<VkDeviceMemory>		   m_vkUniformBuffersMemory  = {};
			VkFormat						   m_vkSwapChainImageFormat;
			VkExtent2D						   m_vkSwapChainExtent;
			SyncObjects						   m_sSyncObjects;

			std::shared_ptr<VKIntegration> m_pIntegration;
			std::shared_ptr<CWindow>       m_pWindow;
			std::vector<std::shared_ptr<Entity::CEntity>>		m_pEntities;
			std::vector<std::shared_ptr<Components::CTransform>>     m_pTransforms;
			std::vector<std::shared_ptr<Components::CMesh>>     m_pMeshes;
			std::vector<std::shared_ptr<Components::CMaterial>> m_pMaterials;
			std::shared_ptr<Entity::CCamera> m_pCamera;

			std::shared_ptr<GUI> m_pGui;
			bool m_bDrawGui = true;
			float m_fFrameTimeCounter = 0.0f;

			uint32_t	   m_iCurrentFrame      = 0;
			const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
			double m_dDeltaTime;

			static bool m_bFramebufferResized;

		};
	}
}