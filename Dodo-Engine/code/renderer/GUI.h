#pragma once
#include "dodopch.h"
#include "common/VKIntegration.h"
#include "VulkanInitializers.h"
#include "entity/Entity.h"
//#include "engine/Engine.h"
#include "VulkanBuffer.h"
#include "common/VKHelpers.h"
#include "components/Material.h"
#include "environment/Input.h"

namespace Dodo
{
	namespace Rendering
	{
		
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask);


		struct UISettings
		{
			bool displayModels = true;
			bool displayLogos = true;
			bool displayBackground = true;
			bool animateLight = false;
			float roughness = 0.25f;
			float metallic = 0.25f;
			std::array<float, 50> frameTimes{};
			float frameTimeMin = 9999.0f, frameTimeMax = 0.0f;
			float lightTimer = 0.0f;
			int frameTimeCycle = 0;
		};

		// ----------------------------------------------------------------------------
		// ImGUI class
		// ----------------------------------------------------------------------------
		class GUI
		{
		public:
			GUI(std::shared_ptr<VKIntegration> integration, Math::Vector3f* cameraPos, Math::Vector3f* cameraRot, float* cameraSpeed)
				: m_pIntegration(integration)
				, m_cameraPos(cameraPos)
				, m_cameraRot(cameraRot)
				, m_cameraSpeed(cameraSpeed)
			{
				device = m_pIntegration->device();
				ImGui::CreateContext();
			}

			DodoError Initialize(VkRenderPass &renderPass, float width, float height);
			VkResult CreateRenderPass(VkFormat swapChainFormat, VkFormat depthFormat);
			VkResult CreateFramebuffers(std::vector<VkImageView> imageViews, VkImageView depthImageView, VkExtent2D extent);
			DodoError NewFrame(double deltaTime, bool updateFrameGraph);
			DodoError CreateCommandBuffer(std::vector<VkFramebuffer> _frameBuffers, VkCommandPool _commandPool);
			bool UpdateBuffers();
			DodoError DrawFrame(
				VkExtent2D extend, 
				VkRenderPass renderPass,
				std::vector<VkFramebuffer> framebuffers,
				double deltaTime = 0.0);

			void Finalize();

			bool shouldDraw() { return m_bUpdated; }

			// UI params are set via push constants
			struct PushConstBlock
			{
				glm::vec2 scale;
				glm::vec2 translate;
			} pushConstBlock;
			std::vector<VkCommandBuffer> m_vkCommandBuffers = {};

			UISettings uiSettings;
		private:
			// Vulkan resources for rendering the UI
			VkSampler			  sampler					= VK_NULL_HANDLE;
			vks::Buffer			  vertexBuffer				= {};
			vks::Buffer			  indexBuffer				= {};
			int32_t				  vertexCount				= 0;
			int32_t				  indexCount				= 0;
			VkDeviceMemory		  fontMemory				= VK_NULL_HANDLE;
			VkImage				  fontImage					= VK_NULL_HANDLE;
			VkImageView			  fontView					= VK_NULL_HANDLE;
			VkPipelineCache		  pipelineCache				= VK_NULL_HANDLE;
			VkPipelineLayout	  pipelineLayout			= VK_NULL_HANDLE;
			VkPipeline			  pipeline					= VK_NULL_HANDLE;
			VkDescriptorPool	  descriptorPool			= VK_NULL_HANDLE;
			VkDescriptorSetLayout descriptorSetLayout		= VK_NULL_HANDLE;
			VkDescriptorSet		  descriptorSet				= VK_NULL_HANDLE;
			VkRenderPass		  renderPass				= VK_NULL_HANDLE;
			std::vector<VkFramebuffer> frameBuffers = {};

			std::shared_ptr<VKIntegration> m_pIntegration;
			Math::Vector3f* m_cameraPos;
			Math::Vector3f* m_cameraRot;
			float* m_cameraSpeed;
			VkDevice device;

			bool m_bUpdated = false;

		};
	}
}



