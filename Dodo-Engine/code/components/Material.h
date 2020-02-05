#pragma once
#include "dodopch.h"
#include "common/VKHelpers.h"
#include "environment/Error.h"
#include "common/DodoTypes.h"
#include "ECS.h"
#include "Mesh.h"
#include <stb/stb_image.h>
#include <variant>

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
				__declspec(align(16)) Math::Matrix4x4 model;
				__declspec(align(16)) Math::Matrix4x4 View;
				__declspec(align(16)) Math::Matrix4x4 projection;

				__declspec(align(16)) Math::Vector3f camPos;
			};

			struct PushConsts
			{
				float roughness = 0.2f;
				float metallic = 0.5f;
				float r, g, b = 1.0f;
			};

			struct Texture
			{
				struct TextureData
				{
					VkImage				  textureImage       = VK_NULL_HANDLE;
					VkDeviceMemory		  textureImageMemory = VK_NULL_HANDLE;
					VkImageView			  textureImageView	 = VK_NULL_HANDLE;
					VkSampler			  textureSampler	 = VK_NULL_HANDLE;
					VkDescriptorImageInfo imageInfo			 = {};
				};


				std::string  filename     = "";
				int			 texWidth	  = 0;
				int			 texHeight    = 0;
				int			 texChannels  = 0;
				int			 mipLevels	  = 1;
				stbi_uc*	 pixels		  = nullptr;
				TextureData  textureData  = {};

			};

			struct Textures
			{
				Texture albedo;
				Texture normal;
				Texture metallic;
				Texture roughness;
			};

			CMaterial(std::shared_ptr<VKIntegration> _integration, ShaderInfo _shaderInfo)
				: m_pIntegration(_integration),
				m_shaderInfo(_shaderInfo)
			{
				m_textures.albedo.filename    = std::string();
				m_textures.normal.filename    = std::string();
				m_textures.metallic.filename  = std::string();
				m_textures.roughness.filename = std::string();

				SetTextures(
					"resources/textures/default.png", 
					"resources/textures/black.png", 
					"resources/textures/black.png", 
					"resources/textures/black.png");
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

			DodoError LoadTexture(Texture &texture);

			//virtual ~CMaterial() = default;

			virtual DodoError Create() { return DODO_OK; }
			virtual DodoError Initialize() { return DODO_OK; }
			virtual void Finalize();
			virtual void Update() {  }
			virtual DodoError Commit() { return DODO_OK; }	

			

			// Getter / Setter
			Shaders const shaders() const { return m_shaders; }
			Textures& const textures() { return m_textures; }
			void SetTextures(std::string _albedo, std::string _normal, std::string _metallic, std::string _roughness)
			{ 
				m_textures.albedo   .filename = _albedo;
				m_textures.normal   .filename = _normal;
				m_textures.metallic .filename = _metallic;
				m_textures.roughness.filename = _roughness;

				LoadTexture(m_textures.albedo);
				LoadTexture(m_textures.normal);
				LoadTexture(m_textures.metallic);
				LoadTexture(m_textures.roughness);
			}

			void SetAlbedoTextureData(Texture _textureData) { m_textures.albedo = _textureData; }

			PushConsts& const pushConstants() { return pushConsts; }
			void setPushConstants(float roughness, float metallic, float r, float g, float b) 
			{
				pushConsts.roughness = roughness;
				pushConsts.metallic = metallic;
				pushConsts.r = r;
				pushConsts.g = g;
				pushConsts.b = b;
			}

			
		protected:
			Shaders     m_shaders	 = {};
			ShaderInfo  m_shaderInfo = {};
			Textures    m_textures	 = {};
			PushConsts  pushConsts   = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

			std::shared_ptr<VKIntegration> m_pIntegration;
		};
	}
}


