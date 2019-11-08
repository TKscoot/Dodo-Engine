#pragma once
#include "entity/Entity.h"
#include "components/Mesh.h"
#include "components/Material.h"
#include "VulkanInitializers.h"
#include <gli/gli/gli.hpp>
#include "entity/Camera.h"

namespace Dodo
{
	namespace Rendering
	{
		using namespace Entity;
		using namespace Environment;
		class CSkybox
		{
		public:
			CSkybox(std::shared_ptr<VKIntegration> _integration, VkRenderPass _renderPass, VkCommandPool _commandPool, VkExtent2D _swapExtent)
				: m_pIntegration(_integration)
				, m_vkRenderPass(_renderPass)
				, m_vkCmdPool(_commandPool)
				, m_vkSwapExtent(_swapExtent)
			{

			}

			DodoError Initialize();
			VkResult  LoadCubemap(std::string _filename, VkFormat _format);
			VkResult  SetDescriptorSetLayout();
			VkResult  SetupDescriptorSet();
			VkResult  CreatePipeline();

			void  BuildOnMainCmdBuf(VkCommandBuffer _cmdBuf)
			{
				VkDeviceSize offsets[1] = { 0 };

				vkCmdBindDescriptorSets(_cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0, 1, &m_vkDescriptorSet, 0, NULL);
				vkCmdBindPipeline(		_cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
				vkCmdBindVertexBuffers(	_cmdBuf, 0, 1, &m_compMesh->m_dataBuffers.vertexBuffer, offsets);
				vkCmdBindIndexBuffer(	_cmdBuf, m_compMesh->m_dataBuffers.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdSetViewport(		_cmdBuf, 0, 1, &viewport);
				vkCmdSetScissor(		_cmdBuf, 0, 1, &scissor);
				vkCmdPushConstants(		_cmdBuf, m_vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Math::Vector3f), &m_compTransform->getPosition());
				vkCmdDrawIndexed(		_cmdBuf, m_compMesh->m_indices.size(), 1, 0, 0, 0);

			}
			VkResult  UpdateUniformBuffer(std::shared_ptr<CCamera> _cam);

			void SetRenderPass(VkRenderPass _rp) { m_vkRenderPass = _rp; }
			void SetExtent(VkExtent2D _ext) { m_vkSwapExtent = _ext; }


			Components::CMaterial::Texture			m_cubeMap;

			std::shared_ptr<Components::CMesh>		m_compMesh;
			std::shared_ptr<Components::CMaterial>  m_compMaterial;

		private:

			void CreateVertIdxBuffers();


			CEntity* m_pSkyboxEnt				    = nullptr;
			std::shared_ptr<Components::CTransform> m_compTransform;
			//std::array<Components::CMaterial::Texture, 6> m_textures;

			// vulkan handles
			VkDescriptorSetLayout m_vkDescriptorSetLayout;
			VkPipelineLayout      m_vkPipelineLayout;
			VkRenderPass          m_vkRenderPass;
			VkPipeline			  m_vkPipeline;
			VkDescriptorPool      m_vkDescriptorPool;
			VkDescriptorSet       m_vkDescriptorSet;
			VkCommandPool		  m_vkCmdPool;
			VkExtent2D			  m_vkSwapExtent;
			VkBuffer			  m_vkUniformBuffer;
			VkDeviceMemory		  m_vkUniformBufferMemory;

			VkViewport			  viewport = {};
			VkRect2D			  scissor = {};

			std::shared_ptr<VKIntegration> m_pIntegration;

		};
	}
}
