#pragma once
#include "dodopch.h"
#include "common/VKIntegration.h"
#include "common/VKHelpers.h"
#include "entity/Entity.h"
#include "components/Material.h"
#include "entity/EntityHandler.h"
#include "entity/Camera.h"

namespace Dodo
{
	namespace Rendering
	{

		using namespace Entity;

		struct SubpassParameters
		{
			VkPipelineBindPoint                  PipelineType;
			std::vector<VkAttachmentReference>   InputAttachments;
			std::vector<VkAttachmentReference>   ColorAttachments;
			std::vector<VkAttachmentReference>   ResolveAttachments;
			VkAttachmentReference const        * DepthStencilAttachment;
			std::vector<uint32_t>                PreserveAttachments;
		};



		struct ViewportInfo
		{
			std::vector<VkViewport>   Viewports;
			std::vector<VkRect2D>     Scissors;
		};

		class CShadowMapping
		{
		public:
			CShadowMapping(std::shared_ptr<VKIntegration> _integration, std::shared_ptr<CCamera> _cam)
			: m_pIntegration(_integration)
			, m_pCamera(_cam)
			{}

			VkResult Initialize();
			VkResult RecordCommandBuffer(VkCommandBuffer _cb);
			VkResult UpdateUniformBuffer();

		private:
			// methods
			VkResult CreateRenderPass();
			VkResult CreateFramebuffer();
			VkResult CreatePipeline();


			// structs
			struct ShadowUbo
			{
				Math::Matrix4x4 light_view_matrix;
				Math::Matrix4x4 scene_view_matrix;
				Math::Matrix4x4 perspective_matrix;
			};

			// vars
			std::shared_ptr<VKIntegration> m_pIntegration;
			std::shared_ptr<CCamera> m_pCamera;

			VkImage			 m_vkImage		   = VK_NULL_HANDLE;
			VkDeviceMemory	 m_vkMemory		   = VK_NULL_HANDLE;
			VkImageView		 m_vkImageView	   = VK_NULL_HANDLE;
			VkFramebuffer	 m_vkFramebuffer   = VK_NULL_HANDLE;
			VkSampler		 m_vkSampler	   = VK_NULL_HANDLE;

			VkRenderPass	 m_vkRenderPass	   = VK_NULL_HANDLE;
			VkPipeline		 m_vkPipeline	   = VK_NULL_HANDLE;
			VkPipelineLayout m_vkPipelineLaout = VK_NULL_HANDLE;

			VkDescriptorSetLayout m_vkDescriptorSetLayout = VK_NULL_HANDLE;
			VkDescriptorPool m_vkDescriptorPool = VK_NULL_HANDLE;
			VkDescriptorSet m_vkDescriptorSet  = VK_NULL_HANDLE;

			VkBuffer m_vkUniformBuffer = VK_NULL_HANDLE;
			VkDeviceMemory m_vkUniformBufferMemory = VK_NULL_HANDLE;


		};
	}
}