#pragma once
#include "dodopch.h"
#include "environment/Error.h"
#include "common/DodoTypes.h"
#include "ECS.h"

namespace Dodo
{
	namespace Components
	{
		using namespace Dodo::Environment;
		using namespace Dodo::Rendering;
		
		class CMaterial : public CComponent
		{
		public:
			static VkVertexInputBindingDescription GetBindingDescription()
			{
				VkVertexInputBindingDescription bindingDescription = {};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 5> GetAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};
				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, position);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, normal);

				attributeDescriptions[2].binding = 0;
				attributeDescriptions[2].location = 2;
				attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[2].offset = offsetof(Vertex, tangent);

				attributeDescriptions[3].binding = 0;
				attributeDescriptions[3].location = 3;
				attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[3].offset = offsetof(Vertex, texcoords);

				attributeDescriptions[4].binding = 0;
				attributeDescriptions[4].location = 4;
				attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[4].offset = offsetof(Vertex, color);

				return attributeDescriptions;
			}

			struct ShaderInfo
			{
				std::string vertexShaderFileName;
				std::string fragmentShaderFileName;
			};

			struct Shaders
			{
				//VkShaderModule					vertShader;
				//VkShaderModule					fragShader;
				VkPipelineShaderStageCreateInfo vertShaderStage;
				VkPipelineShaderStageCreateInfo fragShaderStage;
			};

			struct DataBuffer
			{
				// Vertex buffer
				VkBuffer vertexBuffer;
				VkDeviceMemory vertexBufferMemory;

				// Index buffer
				VkBuffer indexBuffer;
				VkDeviceMemory indexBufferMemory;

				VkBuffer uniformBuffer;
				VkDeviceMemory uniformBufferMemory;
			};

			struct UniformBufferObject
			{
				Math::Matrix4x4 model;
				Math::Matrix4x4 View;
				Math::Matrix4x4 projection;
			};

			CMaterial(std::shared_ptr<VKIntegration> _integration, ShaderInfo _shaderInfo)
			{
				VkShaderModule vertShaderModule = VKHelper::CreateShaderModule(_integration, _shaderInfo.vertexShaderFileName);
				VkShaderModule fragShaderModule = VKHelper::CreateShaderModule(_integration, _shaderInfo.fragmentShaderFileName);

				VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
				vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				vertShaderStageInfo.module = vertShaderModule;
				vertShaderStageInfo.pName = "main";

				VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
				fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				fragShaderStageInfo.module = fragShaderModule;
				fragShaderStageInfo.pName = "main";

				//VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
				m_shaders.vertShaderStage = vertShaderStageInfo;
				m_shaders.fragShaderStage = fragShaderStageInfo;
			}

			virtual ~CMaterial() = default;

			virtual DodoError Create() { return DODO_OK; }
			virtual DodoError Initialize() { return DODO_OK; }
			virtual void Finalize() { }			
			virtual void Update() { }
			virtual DodoError Commit() { return DODO_OK; }	

			

			// Getter / Setter
			Shaders const shaders() const { return m_shaders; }
			const std::vector<Vertex> const vertices() { return m_vertices; }

			std::vector<Vertex> m_vertices = {
				{{-0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
				{{0.5f, -0.5f , 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
				{{0.5f, 0.5f  , 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
				{{-0.5f, 0.5f , 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}
			};

			std::vector<uint32_t> indices = {
				0, 1, 2, 2, 3, 0
			};

			DataBuffer m_dataBuffers;

		protected:
			Shaders m_shaders;

		};

		class TestMaterial : public CMaterial
		{
		public:
			using CMaterial::CMaterial;

			//DodoError Create()     { return DODO_OK; }
			//DodoError Initialize() { return DODO_OK; }
			//DodoError Finalize()   { return DODO_OK; }
			//DodoError Update()     { return DODO_OK; }
			//DodoError Commit()     { return DODO_OK; }
		};
	}
}


