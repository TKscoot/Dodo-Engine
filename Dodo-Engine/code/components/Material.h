#pragma once
#include "dodopch.h"
#include "common/VKHelpers.h"
#include "environment/Error.h"
#include "common/DodoTypes.h"
#include "ECS.h"
#include "Mesh.h"
#include <stb/stb_image.h>

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



			struct UniformBufferObject
			{
				Math::Matrix4x4 model;
				Math::Matrix4x4 View;
				Math::Matrix4x4 projection;
			};

			struct TextureData
			{
				struct TextureImage
				{
					VkImage		   textureImage		  = VK_NULL_HANDLE;
					VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
					VkImageView	   textureImageView   = VK_NULL_HANDLE;
					VkSampler	   textureSampler	  = VK_NULL_HANDLE;
				};

				std::string  filename     = "";
				int			 texWidth	  = 0;
				int			 texHeight    = 0;
				int			 texChannels  = 0;
				stbi_uc*	 pixels		  = nullptr;
				TextureImage textureImage = {};

			};

			CMaterial(std::shared_ptr<VKIntegration> _integration, ShaderInfo _shaderInfo)
				: m_pIntegration(_integration),
				m_shaderInfo(_shaderInfo)
			{
				m_texture.filename = std::string();
			}

			DodoError CreateShaders()
			{
				// ShaderModules im renderer oder so erstellen, sonst braucht man hier VKIntegration...
				VkShaderModule vertShaderModule = Dodo::Rendering::VKHelper::CreateShaderModule(m_pIntegration, m_shaderInfo.vertexShaderFileName);
				VkShaderModule fragShaderModule = Dodo::Rendering::VKHelper::CreateShaderModule(m_pIntegration, m_shaderInfo.fragmentShaderFileName);

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

				return DodoError::DODO_OK;
			}

			DodoError LoadTexture();

			//virtual ~CMaterial() = default;

			virtual DodoError Create() { return DODO_OK; }
			virtual DodoError Initialize() { return DODO_OK; }
			virtual void Finalize();
			virtual void Update() { }
			virtual DodoError Commit() { return DODO_OK; }	

			

			// Getter / Setter
			Shaders const shaders() const { return m_shaders; }
			TextureData& const textureData() { return m_texture; }
			void SetTexture(std::string _filename) 
			{ 
				m_texture.filename = _filename;
				LoadTexture();
			}

		protected:
			Shaders     m_shaders	 = {};
			ShaderInfo  m_shaderInfo = {};
			TextureData m_texture	 = {};

			std::shared_ptr<VKIntegration> m_pIntegration;
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


