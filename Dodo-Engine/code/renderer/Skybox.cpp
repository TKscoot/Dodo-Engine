#include "dodopch.h"
#include "Skybox.h"

namespace Dodo
{
	namespace Rendering
	{
		using namespace Entity;
		using namespace Environment;
		DodoError CSkybox::Initialize()
		{
			m_pSkyboxEnt = new CEntity("Skybox");

			Components::CMaterial::ShaderInfo shaderInfo = {};
			shaderInfo.vertexShaderFileName = "shaders/skybox.vert.spv";
			shaderInfo.fragmentShaderFileName = "shaders/skybox.frag.spv";
			m_compMaterial = m_pSkyboxEnt->AddComponent<Components::CMaterial>(m_pIntegration, shaderInfo);

			m_compMesh      = m_pSkyboxEnt->AddComponent<Components::CMesh>();
			m_compMesh->CreateMeshFromFile("resources/models/cube.obj");

			m_compTransform = m_pSkyboxEnt->AddComponent<Components::CTransform>();
			m_compTransform->setPosition(1.0f, 1.0f, 1.0f);
			m_compTransform->setRotation(1.0f, 1.0f, 1.0f);
			m_compTransform->setScale(1.0f, 1.0f, 1.0f);


			


			//LoadCubemap("resources/textures/cubemap_yokohama_bc3_unorm.ktx", VK_FORMAT_BC3_UNORM_BLOCK);
			LoadCubemap("resources/textures/UnitySkybox/OutputCube.dds", VK_FORMAT_B8G8R8A8_UNORM);

			CreateVertIdxBuffers();
			SetDescriptorSetLayout();
			SetupDescriptorSet();
			CreatePipeline();



			return DodoError::DODO_OK;
		}

		VkResult CSkybox::LoadCubemap(std::string _filename, VkFormat _format)
		{
			VkResult result = VK_ERROR_INITIALIZATION_FAILED;

			std::array<gli::texture2d, 6> gliTextures;
			std::array<Components::CMaterial::Texture, 6> cubeTextures;
			cubeTextures[0].filename = "resources/textures/UnitySkybox/sky8_RT.pgm.ktx";
			cubeTextures[1].filename = "resources/textures/UnitySkybox/sky8_LF.pgm.ktx";
			cubeTextures[2].filename = "resources/textures/UnitySkybox/sky8_UP.pgm.ktx";
			cubeTextures[3].filename = "resources/textures/UnitySkybox/sky8_DN.pgm.ktx";
			cubeTextures[4].filename = "resources/textures/UnitySkybox/sky8_BK.pgm.ktx";
			cubeTextures[5].filename = "resources/textures/UnitySkybox/sky8_FR.pgm.ktx";
			
			for (int i = 0; i < cubeTextures.size(); i++)
			{
				int texWidth, texHeight, texChannels;
			
			
				stbi_uc* pixels = stbi_load(cubeTextures[i].filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

				gli::texture2d tex = gli::texture2d(gli::load(cubeTextures[i].filename));
				gliTextures[i] = tex;


			
				if (!pixels)
				{
					CLog::Error("Failed to load texture image!");
				}
			
			
				cubeTextures[i].texWidth = texWidth;
				cubeTextures[i].texHeight = texHeight;
				cubeTextures[i].texChannels = texChannels;
				cubeTextures[i].pixels = pixels;
				cubeTextures[i].mipLevels = 1;
			}

			gli::texture_cube texCube(gli::load(_filename));
			//texCube[0] = gliTextures[0];
			//texCube[1] = gliTextures[1];
			//texCube[2] = gliTextures[2];
			//texCube[3] = gliTextures[3];
			//texCube[4] = gliTextures[4];
			//texCube[5] = gliTextures[5];

			assert(!texCube.empty());

			m_cubeMap.pixels = (stbi_uc*)texCube.data();
			m_cubeMap.texWidth = texCube.extent().x;
			m_cubeMap.texHeight = texCube.extent().y;
			m_cubeMap.mipLevels = texCube.levels();

			VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
			VkMemoryRequirements memReqs;

			// Create a host-visible staging buffer that contains the raw image data
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingMemory;

			VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
			bufferCreateInfo.size = texCube.size();

			// This buffer is used as a transfer source for the buffer copy
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			CError::CheckError<VkResult>(vkCreateBuffer(m_pIntegration->device(), &bufferCreateInfo, nullptr, &stagingBuffer));

			// Get memory requirements for the staging buffer (alignment, memory type bits)
			vkGetBufferMemoryRequirements(m_pIntegration->device(), stagingBuffer, &memReqs);
			memAllocInfo.allocationSize = memReqs.size;
			// Get memory type index for a host visible buffer
			memAllocInfo.memoryTypeIndex = m_pIntegration->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			CError::CheckError<VkResult>(vkAllocateMemory(m_pIntegration->device(), &memAllocInfo, nullptr, &stagingMemory));
			CError::CheckError<VkResult>(vkBindBufferMemory(m_pIntegration->device(), stagingBuffer, stagingMemory, 0));

			// Copy texture data into staging buffer
			uint8_t *data;
			CError::CheckError<VkResult>(vkMapMemory(m_pIntegration->device(), stagingMemory, 0, memReqs.size, 0, (void **)&data));
			memcpy(data, texCube.data(), texCube.size());
			vkUnmapMemory(m_pIntegration->device(), stagingMemory);

			// Create optimal tiled target image
			VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
			imageCreateInfo.imageType	  = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format		  = _format;
			imageCreateInfo.mipLevels	  = texCube.levels();
			imageCreateInfo.samples		  = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling		  = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.sharingMode	  = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.extent		  = { static_cast<uint32_t>(m_cubeMap.texWidth), static_cast<uint32_t>(m_cubeMap.texHeight), 1 };
			imageCreateInfo.usage		  = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			// Cube faces count as array layers in Vulkan
			imageCreateInfo.arrayLayers	  = 6;
			// This flag is required for cube map images
			imageCreateInfo.flags		  = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

			CError::CheckError<VkResult>(vkCreateImage(m_pIntegration->device(), &imageCreateInfo, nullptr, &m_cubeMap.textureData.textureImage));

			vkGetImageMemoryRequirements(m_pIntegration->device(), m_cubeMap.textureData.textureImage, &memReqs);

			memAllocInfo.allocationSize  = memReqs.size;
			memAllocInfo.memoryTypeIndex = m_pIntegration->FindMemoryType(
															memReqs.memoryTypeBits, 
															VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			result = vkAllocateMemory(
				m_pIntegration->device(), 
				&memAllocInfo, 
				nullptr, 
				&m_cubeMap.textureData.textureImageMemory);
			
			result = vkBindImageMemory(
					m_pIntegration->device(), 
					m_cubeMap.textureData.textureImage, 
					m_cubeMap.textureData.textureImageMemory, 
					0);

			//m_vkCmdPool = VKHelper::createCommandPool(m_pIntegration, m_pIntegration->queueFamilies().graphicsQueueFamilyIndices);
			VkCommandBuffer copyCmd = VKHelper::createCommandBuffer(m_pIntegration, VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_vkCmdPool, true);

			// Setup buffer copy regions for each face including all of it's miplevels
			std::vector<VkBufferImageCopy> bufferCopyRegions;
			uint32_t offset = 0;

			for (uint32_t face = 0; face < texCube.faces(); face++)
			{
				for (uint32_t level = 0; level < texCube.levels(); level++)
				{
					VkBufferImageCopy bufferCopyRegion = {};
					bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					bufferCopyRegion.imageSubresource.mipLevel = level;
					bufferCopyRegion.imageSubresource.baseArrayLayer = face;
					bufferCopyRegion.imageSubresource.layerCount = 1;
					bufferCopyRegion.imageExtent.width = texCube[face][level].extent().x;
					bufferCopyRegion.imageExtent.height = texCube[face][level].extent().y;
					bufferCopyRegion.imageExtent.depth = 1;
					bufferCopyRegion.bufferOffset = offset;

					bufferCopyRegions.push_back(bufferCopyRegion);

					// Increase offset into staging buffer for next level / face
					offset += texCube[face][level].size();
				}
			}

			// Image barrier for optimal image (target)
			// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount   = texCube.levels();
			subresourceRange.layerCount   = 6;

			VKHelper::setImageLayout(
				copyCmd,
				m_cubeMap.textureData.textureImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange,
				VK_PIPELINE_STAGE_HOST_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT);

			// Copy the cube map faces from the staging buffer to the optimal tiled image
			vkCmdCopyBufferToImage(
				copyCmd,
				stagingBuffer,
				m_cubeMap.textureData.textureImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<uint32_t>(bufferCopyRegions.size()),
				bufferCopyRegions.data()
			);

			// Change texture image layout to shader read after all faces have been copied
			m_cubeMap.textureData.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VKHelper::setImageLayout(
				copyCmd,
				m_cubeMap.textureData.textureImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresourceRange,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

			VKHelper::flushCommandBuffer(m_pIntegration, copyCmd, m_pIntegration->queues().graphicsQueue, m_vkCmdPool, true);

			// Create sampler
			VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
			sampler.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler.magFilter				= VK_FILTER_LINEAR;
			sampler.minFilter				= VK_FILTER_LINEAR;
			sampler.addressModeU			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeV			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeW			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.unnormalizedCoordinates = VK_FALSE;
			sampler.compareEnable			= VK_FALSE;
			sampler.compareOp				= VK_COMPARE_OP_ALWAYS;
			sampler.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler.mipLodBias				= 0.0f;
			sampler.minLod					= 0.0f;
			sampler.maxLod					= texCube.levels();
			sampler.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			sampler.anisotropyEnable		= VK_TRUE;
			sampler.maxAnisotropy			= 16;

			CError::CheckError<VkResult>(vkCreateSampler(m_pIntegration->device(), &sampler, nullptr, &m_cubeMap.textureData.textureSampler));

			// Create image view
			VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
			// Cube map view type
			view.viewType					 = VK_IMAGE_VIEW_TYPE_CUBE;
			view.format						 = _format;
			view.components					 = { 
												VK_COMPONENT_SWIZZLE_R, 
												VK_COMPONENT_SWIZZLE_G, 
												VK_COMPONENT_SWIZZLE_B, 
												VK_COMPONENT_SWIZZLE_A };
			view.subresourceRange			 = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			// 6 array layers (faces)
			view.subresourceRange.layerCount = 6;
			// Set number of mip levels
			view.subresourceRange.levelCount = texCube.levels();
			view.image = m_cubeMap.textureData.textureImage;
			
			CError::CheckError<VkResult>(vkCreateImageView(m_pIntegration->device(), &view, nullptr, &m_cubeMap.textureData.textureImageView));

			// Clean up staging resources
			vkFreeMemory(m_pIntegration->device(), stagingMemory, nullptr);
			vkDestroyBuffer(m_pIntegration->device(), stagingBuffer, nullptr);
			
			return result;
		}

		VkResult CSkybox::SetDescriptorSetLayout()
		{
			VkResult result = VK_ERROR_INITIALIZATION_FAILED;

			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
			{
				// Binding 0 : Vertex shader uniform buffer
				vks::initializers::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					VK_SHADER_STAGE_VERTEX_BIT,
					0),
				// Binding 1 : Fragment shader image sampler
				vks::initializers::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					1)
			};

			VkDescriptorSetLayoutCreateInfo descriptorLayout =
				vks::initializers::descriptorSetLayoutCreateInfo(
					setLayoutBindings.data(),
					setLayoutBindings.size());

			result = vkCreateDescriptorSetLayout(m_pIntegration->device(), &descriptorLayout, nullptr, &m_vkDescriptorSetLayout);

			std::vector<VkPushConstantRange> pushConstantRanges = {
				vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::vec3), 0)
			};

			VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
				vks::initializers::pipelineLayoutCreateInfo(
					&m_vkDescriptorSetLayout,
					1);

			pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
			pPipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

			result = vkCreatePipelineLayout(m_pIntegration->device(), &pPipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout);

			return result;
		}

		VkResult CSkybox::SetupDescriptorSet()
		{
			VkResult result = VK_ERROR_INITIALIZATION_FAILED;

			// Create descriptor pool
			std::vector<VkDescriptorPoolSize> poolSizes =
			{
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo =
				vks::initializers::descriptorPoolCreateInfo(
					poolSizes.size(),
					poolSizes.data(),
					2);

			result = vkCreateDescriptorPool(m_pIntegration->device(), &descriptorPoolInfo, nullptr, &m_vkDescriptorPool);
			CError::CheckError<VkResult>(result);

			VkDescriptorBufferInfo uboDescriptor = {};
			uboDescriptor.buffer = m_vkUniformBuffer;
			uboDescriptor.offset = 0;
			uboDescriptor.range = sizeof(Components::CMaterial::UniformBufferObject);
			


			// Image descriptor for the cube map texture
			VkDescriptorImageInfo textureDescriptor =
				vks::initializers::descriptorImageInfo(
					m_cubeMap.textureData.textureSampler,
					m_cubeMap.textureData.textureImageView,
					m_cubeMap.textureData.imageInfo.imageLayout);

			VkDescriptorSetAllocateInfo allocInfo =
				vks::initializers::descriptorSetAllocateInfo(
					m_vkDescriptorPool,
					&m_vkDescriptorSetLayout,
					1);

			
			// Sky box descriptor set
			result = vkAllocateDescriptorSets(m_pIntegration->device(), &allocInfo, &m_vkDescriptorSet);
			CError::CheckError<VkResult>(result);


			std::vector<VkWriteDescriptorSet> writeDescriptorSets =
			{
				// Binding 0 : Vertex shader uniform buffer
				vks::initializers::writeDescriptorSet(
					m_vkDescriptorSet,
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					0,
					&uboDescriptor),
				// Binding 1 : Fragment shader cubemap sampler
				vks::initializers::writeDescriptorSet(
					m_vkDescriptorSet,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					&textureDescriptor)
			};

			vkUpdateDescriptorSets(
				m_pIntegration->device(), 
				static_cast<uint32_t>(writeDescriptorSets.size()), 
				writeDescriptorSets.data(), 
				0, 
				nullptr);

			return result;
		}

		VkResult CSkybox::CreatePipeline()
		{
			VkResult result = VK_ERROR_INITIALIZATION_FAILED;

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
				vks::initializers::pipelineInputAssemblyStateCreateInfo(
					VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
					0,
					VK_FALSE);

			VkPipelineRasterizationStateCreateInfo rasterizationState =
				vks::initializers::pipelineRasterizationStateCreateInfo(
					VK_POLYGON_MODE_FILL,
					VK_CULL_MODE_FRONT_BIT,
					VK_FRONT_FACE_COUNTER_CLOCKWISE,
					0);

			VkPipelineColorBlendAttachmentState blendAttachmentState =
				vks::initializers::pipelineColorBlendAttachmentState(
					0xf,
					VK_FALSE);

			VkPipelineColorBlendStateCreateInfo colorBlendState =
				vks::initializers::pipelineColorBlendStateCreateInfo(
					1,
					&blendAttachmentState);

			VkPipelineDepthStencilStateCreateInfo depthStencilState =
				vks::initializers::pipelineDepthStencilStateCreateInfo(
					VK_FALSE,
					VK_FALSE,
					VK_COMPARE_OP_LESS_OR_EQUAL);

			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)m_vkSwapExtent.width;
			viewport.height = (float)m_vkSwapExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			scissor.offset = { 0, 0 };
			scissor.extent = m_vkSwapExtent;

			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;


			VkPipelineMultisampleStateCreateInfo multisampleState =
				vks::initializers::pipelineMultisampleStateCreateInfo(
					VK_SAMPLE_COUNT_1_BIT,
					0);

			std::vector<VkDynamicState> dynamicStateEnables = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			VkPipelineDynamicStateCreateInfo dynamicState =
				vks::initializers::pipelineDynamicStateCreateInfo(
					dynamicStateEnables.data(),
					dynamicStateEnables.size(),
					0);

			// Vertex bindings and attributes
			//VkVertexInputBindingDescription vertexInputBinding = m_compMaterial->GetBindingDescription();
			//
			//std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			//	vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Location 0: Position
			//	vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Math::Vector3f)),	// Location 1: Normal
			//};
			//
			//VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
			//vertexInputState.vertexBindingDescriptionCount	 = 1;
			//vertexInputState.pVertexBindingDescriptions		 = &vertexInputBinding;
			//vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
			//vertexInputState.pVertexAttributeDescriptions	 = vertexInputAttributes.data();

			auto bindingDescriptions   = Components::CMaterial::GetBindingDescription();
			auto attributeDescriptions = Components::CMaterial::GetAttributeDescriptions();

			VkPipelineVertexInputStateCreateInfo vertexInputState = {};
			vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputState.vertexBindingDescriptionCount = 1;
			vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputState.pVertexBindingDescriptions = &bindingDescriptions;
			vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

			std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
			m_compMaterial->CreateShaders();
			shaderStages[0] = m_compMaterial->shaders().fragShaderStage;
			shaderStages[1] = m_compMaterial->shaders().vertShaderStage;

			VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(m_vkPipelineLayout, m_vkRenderPass, 0);
			pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
			pipelineCreateInfo.pRasterizationState = &rasterizationState;
			pipelineCreateInfo.pColorBlendState	   = &colorBlendState;
			pipelineCreateInfo.pMultisampleState   = &multisampleState;
			pipelineCreateInfo.pViewportState	   = &viewportState;
			pipelineCreateInfo.pDepthStencilState  = &depthStencilState;
			pipelineCreateInfo.flags			   = VK_DYNAMIC_STATE_SCISSOR | VK_DYNAMIC_STATE_VIEWPORT;
			pipelineCreateInfo.pDynamicState	   = &dynamicState;
			pipelineCreateInfo.stageCount		   = shaderStages.size();
			pipelineCreateInfo.pStages			   = shaderStages.data();
			pipelineCreateInfo.pVertexInputState   = &vertexInputState;


			// Skybox pipeline (background cube)
			result = vkCreateGraphicsPipelines(m_pIntegration->device(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_vkPipeline);
			CError::CheckError<VkResult>(result);
			
			return result;
		}

		void CSkybox::CreateVertIdxBuffers()
		{
			// Vertex Buffer
			const std::vector<Vertex> verts = m_compMesh->vertices();
			VkDeviceSize vertBufferSize = sizeof(verts[0]) * verts.size();

			// creating staging buffer
			VkBuffer vertStagingBuffer;
			VkDeviceMemory vertStagingBufferMemory;

			VKHelper::CreateBuffer(
				m_pIntegration,
				vertBufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				vertStagingBuffer, 
				vertStagingBufferMemory);

			// copying vert data to staging buffer
			void* vertData;
			vkMapMemory(m_pIntegration->device(), vertStagingBufferMemory, 0, vertBufferSize, 0, &vertData);
			memcpy(vertData, verts.data(), (size_t)vertBufferSize);
			vkUnmapMemory(m_pIntegration->device(), vertStagingBufferMemory);

			// creating vertex Buffer
			VKHelper::CreateBuffer(
				m_pIntegration,
				vertBufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_compMesh->m_dataBuffers.vertexBuffer, 
				m_compMesh->m_dataBuffers.vertexBufferMemory);

			// copying staging buffer to vertex buffer
			VKHelper::CopyBuffer(
				m_pIntegration,
				m_vkCmdPool,
				vertStagingBuffer, 
				m_compMesh->m_dataBuffers.vertexBuffer, 
				vertBufferSize);

			vkDestroyBuffer(m_pIntegration->device(), vertStagingBuffer, nullptr);
			vkFreeMemory(m_pIntegration->device(), vertStagingBufferMemory, nullptr);

			// Index Buffer
			VkDeviceSize idxBufferSize = sizeof(m_compMesh->m_indices[0]) * m_compMesh->m_indices.size();

			VkBuffer idxStagingBuffer;
			VkDeviceMemory idxStagingBufferMemory;
			VKHelper::CreateBuffer(
				m_pIntegration, 
				idxBufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				idxStagingBuffer,
				idxStagingBufferMemory);

			void* idxData;
			vkMapMemory(m_pIntegration->device(), idxStagingBufferMemory, 0, idxBufferSize, 0, &idxData);
			memcpy(idxData, m_compMesh->m_indices.data(), (size_t)idxBufferSize);
			vkUnmapMemory(m_pIntegration->device(), idxStagingBufferMemory);

			VKHelper::CreateBuffer(
				m_pIntegration,
				idxBufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_compMesh->m_dataBuffers.indexBuffer, m_compMesh->m_dataBuffers.indexBufferMemory);

			VKHelper::CopyBuffer(
				m_pIntegration, 
				m_vkCmdPool, 
				idxStagingBuffer,
				m_compMesh->m_dataBuffers.indexBuffer, 
				idxBufferSize);

			vkDestroyBuffer(m_pIntegration->device(), idxStagingBuffer, nullptr);
			vkFreeMemory(m_pIntegration->device(), idxStagingBufferMemory, nullptr);

			//m_matDataBuffers.push_back(m_compMesh->m_dataBuffers);

			VkDeviceSize bufferSize = sizeof(Components::CMaterial::UniformBufferObject);

			VKHelper::CreateBuffer(
				m_pIntegration,
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_vkUniformBuffer,
				m_vkUniformBufferMemory);
		}

		VkResult CSkybox::UpdateUniformBuffer(std::shared_ptr<CCamera> _cam)
		{
			VkResult result = VK_ERROR_INITIALIZATION_FAILED;

			Components::CMaterial::UniformBufferObject ubo = {};
			if (m_compTransform != nullptr)
			{
				Math::Matrix4x4 viewMatrix = _cam->getViewMatrix();
				ubo.model = glm::mat4(1.0f);
				ubo.model = viewMatrix * glm::translate(ubo.model, _cam->cameraPos);
			}

			ubo.camPos			  = _cam->cameraPos;
			ubo.View			  = _cam->getViewMatrix();
			ubo.projection		  = _cam->getProjectionMatrix();
			ubo.projection[1][1] *= -1;

			void* data;
			result = vkMapMemory(m_pIntegration->device(), m_vkUniformBufferMemory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(m_pIntegration->device(), m_vkUniformBufferMemory);

			return VK_SUCCESS;
		}
	}
}