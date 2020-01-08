#pragma once
#include "dodopch.h"
#include "common/VKIntegration.h"
#include "common/VKHelpers.h"
#include "entity/Entity.h"
#include "components/Material.h"

namespace Dodo
{
	namespace Rendering
	{

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
			CShadowMapping(std::shared_ptr<VKIntegration> _integration)
			: m_pIntegration(_integration)
			{}

			VkResult Initialize();

		private:
			// methods
			VkResult CreateRenderPass();
			VkResult CreateFramebuffer();
			VkResult CreatePipeline();

			// vars
			std::shared_ptr<VKIntegration> m_pIntegration;

			VkImage			m_vkImage		= VK_NULL_HANDLE;
			VkDeviceMemory	m_vkMemory		= VK_NULL_HANDLE;
			VkImageView		m_vkImageView	= VK_NULL_HANDLE;
			VkFramebuffer	m_vkFramebuffer = VK_NULL_HANDLE;
			VkSampler		m_vkSampler		= VK_NULL_HANDLE;

			VkRenderPass	m_vkRenderPass	= VK_NULL_HANDLE;
			VkPipeline		m_vkPipeline	= VK_NULL_HANDLE;

		};
	}
}