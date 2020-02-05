#include "dodopch.h"
#include "ShadowMapping.h"

VkResult Dodo::Rendering::CShadowMapping::Initialize()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VKHelper::CreateCombinedImageSampler(
		m_pIntegration->physicalDevice(),
		m_pIntegration->device(),
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_D16_UNORM,
		{ 512, 512, 1 },
		1,
		1,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		false,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_SAMPLER_MIPMAP_MODE_NEAREST,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		0.0f,
		false,
		1.0f,
		false,
		VK_COMPARE_OP_ALWAYS,
		0.0f,
		1.0f,
		VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		false,
		m_vkSampler,
		m_vkImage,
		m_vkMemory,
		m_vkImageView);

	CreateRenderPass();
	CreateFramebuffer();
	CreatePipeline();

	return result;
}

VkResult Dodo::Rendering::CShadowMapping::CreateRenderPass()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	std::vector<VkAttachmentDescription> shadowmapAttachmentDescriptions = {
	  {
		0,                                                // VkAttachmentDescriptionFlags     flags
		VK_FORMAT_D16_UNORM,                              // VkFormat                         format
		VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
		VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
		VK_ATTACHMENT_STORE_OP_STORE,                     // VkAttachmentStoreOp              storeOp
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
		VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
		VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL   // VkImageLayout                    finalLayout
	  }
	};

	VkAttachmentReference shadowmapDepthAttachment = {
	  0,                                                // uint32_t                             attachment
	  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  // VkImageLayout                        layout
	};

	std::vector<SubpassParameters> shadowmapSubpassParameters = {
	  {
		VK_PIPELINE_BIND_POINT_GRAPHICS,              // VkPipelineBindPoint                  PipelineType
		{},                                           // std::vector<VkAttachmentReference>   InputAttachments
		{},                                           // std::vector<VkAttachmentReference>   ColorAttachments
		{},                                           // std::vector<VkAttachmentReference>   ResolveAttachments
		&shadowmapDepthAttachment,                 // VkAttachmentReference const        * DepthStencilAttachment
		{}                                            // std::vector<uint32_t>                PreserveAttachments
	  }
	};

	std::vector<VkSubpassDependency> shadowmapSubpassDependencies = {
	  {
		VK_SUBPASS_EXTERNAL,                            // uint32_t                   srcSubpass
		0,                                              // uint32_t                   dstSubpass
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,          // VkPipelineStageFlags       srcStageMask
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,     // VkPipelineStageFlags       dstStageMask
		VK_ACCESS_SHADER_READ_BIT,                      // VkAccessFlags              srcAccessMask
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,   // VkAccessFlags              dstAccessMask
		VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags          dependencyFlags
	  },
	  {
		0,                                              // uint32_t                   srcSubpass
		VK_SUBPASS_EXTERNAL,                            // uint32_t                   dstSubpass
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,      // VkPipelineStageFlags       srcStageMask
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,          // VkPipelineStageFlags       dstStageMask
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,   // VkAccessFlags              srcAccessMask
		VK_ACCESS_SHADER_READ_BIT,                      // VkAccessFlags              dstAccessMask
		VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags          dependencyFlags
	  }
	};


	std::vector<VkSubpassDescription> subpassDescriptions;

	subpassDescriptions.clear();

	for (auto & subpass_description : shadowmapSubpassParameters)
	{
		subpassDescriptions.push_back({
		  0,                                                                      // VkSubpassDescriptionFlags        flags
		  subpass_description.PipelineType,                                       // VkPipelineBindPoint              pipelineBindPoint
		  static_cast<uint32_t>(subpass_description.InputAttachments.size()),     // uint32_t                         inputAttachmentCount
		  subpass_description.InputAttachments.data(),                            // const VkAttachmentReference    * pInputAttachments
		  static_cast<uint32_t>(subpass_description.ColorAttachments.size()),     // uint32_t                         colorAttachmentCount
		  subpass_description.ColorAttachments.data(),                            // const VkAttachmentReference    * pColorAttachments
		  subpass_description.ResolveAttachments.data(),                          // const VkAttachmentReference    * pResolveAttachments
		  subpass_description.DepthStencilAttachment,                             // const VkAttachmentReference    * pDepthStencilAttachment
		  static_cast<uint32_t>(subpass_description.PreserveAttachments.size()),  // uint32_t                         preserveAttachmentCount
		  subpass_description.PreserveAttachments.data()                          // const uint32_t                 * pPreserveAttachments
		});
	}


	VkRenderPassCreateInfo render_pass_create_info = {
	  VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,						   // VkStructureType                    sType
	  nullptr,															   // const void                       * pNext
	  0,																   // VkRenderPassCreateFlags            flags
	  static_cast<uint32_t>(shadowmapAttachmentDescriptions.size()),    // uint32_t                           attachmentCount
	  shadowmapAttachmentDescriptions.data(),                           // const VkAttachmentDescription    * pAttachments
	  static_cast<uint32_t>(subpassDescriptions.size()),				   // uint32_t                           subpassCount
	  subpassDescriptions.data(),										   // const VkSubpassDescription       * pSubpasses
	  static_cast<uint32_t>(shadowmapSubpassDependencies.size()),       // uint32_t                           dependencyCount
	  shadowmapSubpassDependencies.data()                               // const VkSubpassDependency        * pDependencies
	};

	result = vkCreateRenderPass(m_pIntegration->device(), &render_pass_create_info, nullptr, &m_vkRenderPass);
	CError::CheckError<VkResult>(result);


	return result;
}

VkResult Dodo::Rendering::CShadowMapping::CreateFramebuffer()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	std::vector<VkImageView> attachments = { m_vkImageView };

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType		   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass	   = m_vkRenderPass;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments    = attachments.data();
	createInfo.width		   = 512;
	createInfo.height		   = 512;
	createInfo.layers		   = 1;

	result = vkCreateFramebuffer(m_pIntegration->device(), &createInfo, nullptr, &m_vkFramebuffer);
	CError::CheckError<VkResult>(result);

	return result;
}

VkResult Dodo::Rendering::CShadowMapping::CreatePipeline()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	Entity::CEntity* entity = new Entity::CEntity("Shadow");

	Components::CMaterial::ShaderInfo shaderInfo;
	shaderInfo.vertexShaderFileName   = "shaders/shadow.vert.spv";
	shaderInfo.fragmentShaderFileName = "shaders/ui.frag.spv";
	auto mat = entity->AddComponent<Components::CMaterial>(m_pIntegration, shaderInfo);
	mat->CreateShaders();

	std::vector<VkVertexInputBindingDescription> shadowVertexInputBindingDescriptions = {
		{
			0,								// uint32_t                     binding
			6 * sizeof(float),				// uint32_t                     stride
			VK_VERTEX_INPUT_RATE_VERTEX		// VkVertexInputRate            inputRate
		}
	};

	std::vector<VkVertexInputAttributeDescription> shadowVertexAttributeDescriptions = {
		{
			0,                              // uint32_t   location
			0,                              // uint32_t   binding
			VK_FORMAT_R32G32B32_SFLOAT,     // VkFormat   format
			0                               // uint32_t   offset
		},
	};

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo.stage				  = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo.module			  = mat->shaders().vertShaderStage.module;
	shaderStageCreateInfo.pName				  = "main";
	shaderStageCreateInfo.pSpecializationInfo = nullptr;


	auto bindingDescriptions = Components::CMaterial::GetBindingDescription();
	auto attributeDescriptions = Components::CMaterial::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertInputStateCreateInfo = {};
	vertInputStateCreateInfo.sType							 = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	//vertInputStateCreateInfo.vertexBindingDescriptionCount	 = shadowVertexInputBindingDescriptions.size();
	//vertInputStateCreateInfo.pVertexBindingDescriptions		 = shadowVertexInputBindingDescriptions.data();
	//vertInputStateCreateInfo.vertexAttributeDescriptionCount = shadowVertexAttributeDescriptions.size();
	//vertInputStateCreateInfo.pVertexAttributeDescriptions    = shadowVertexAttributeDescriptions.data();

	vertInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescriptions;
	vertInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();



	// Pipeline layout

	std::vector<VkPushConstantRange> push_constant_ranges = {
	  {
		VK_SHADER_STAGE_VERTEX_BIT,     // VkShaderStageFlags     stageFlags
		0,                              // uint32_t               offset
		sizeof(float) * 4             // uint32_t               size
	  }
	};
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
		{
			0,                                          // uint32_t             binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType     descriptorType
			1,                                          // uint32_t             descriptorCount
			VK_SHADER_STAGE_VERTEX_BIT,                 // VkShaderStageFlags   stageFlags
			nullptr                                     // const VkSampler	   *pImmutableSamplers
		},
		{
			1,                                          // uint32_t             binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // VkDescriptorType     descriptorType
			1,                                          // uint32_t             descriptorCount
			VK_SHADER_STAGE_FRAGMENT_BIT,               // VkShaderStageFlags   stageFlags
			nullptr                                     // const VkSampler     *pImmutableSamplers
		}
	};
	VkDescriptorSetLayoutCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = descriptorSetLayoutBindings.size();
	ci.pBindings = descriptorSetLayoutBindings.data();

	result = vkCreateDescriptorSetLayout(m_pIntegration->device(), &ci, nullptr, &m_vkDescriptorSetLayout);


	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType     type
			1                                           // uint32_t             descriptorCount
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // VkDescriptorType     type
			1                                           // uint32_t             descriptorCount
		}
	};
	VkDescriptorPoolCreateInfo dpci = {};
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.poolSizeCount = descriptorPoolSizes.size();
	dpci.pPoolSizes = descriptorPoolSizes.data();
	dpci.maxSets = 1;
	dpci.flags = 0;

	result = vkCreateDescriptorPool(m_pIntegration->device(), &dpci, nullptr, &m_vkDescriptorPool);

	VkDescriptorSetAllocateInfo dsAllocInfo = {};
	dsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsAllocInfo.descriptorPool = m_vkDescriptorPool;
	dsAllocInfo.descriptorSetCount = 1;
	dsAllocInfo.pSetLayouts = &m_vkDescriptorSetLayout;

	result = vkAllocateDescriptorSets(m_pIntegration->device(), &dsAllocInfo, &m_vkDescriptorSet);

	result = VKHelper::CreateBuffer(m_pIntegration, sizeof(Math::Matrix4x4),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_vkUniformBuffer,
		m_vkUniformBufferMemory);

	VKHelper::BufferDescriptorInfo buffer_descriptor_update = {
	  m_vkDescriptorSet,                                // VkDescriptorSet                      TargetDescriptorSet
	  0,                                                // uint32_t                             TargetDescriptorBinding
	  0,                                                // uint32_t                             TargetArrayElement
	  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                // VkDescriptorType                     TargetDescriptorType
	  {                                                 // std::vector<VkDescriptorBufferInfo>  BufferInfos
		{
		  m_vkUniformBuffer,                            // VkBuffer                             buffer
		  0,                                            // VkDeviceSize                         offset
		  VK_WHOLE_SIZE                                 // VkDeviceSize                         range
		}
	  }
	};

	VKHelper::ImageDescriptorInfo image_descriptor_update = {
	  m_vkDescriptorSet,                                  // VkDescriptorSet                      TargetDescriptorSet
	  1,                                                  // uint32_t                             TargetDescriptorBinding
	  0,                                                  // uint32_t                             TargetArrayElement
	  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,          // VkDescriptorType                     TargetDescriptorType
	  {                                                   // std::vector<VkDescriptorImageInfo>   ImageInfos
		{
		  m_vkSampler,									  // VkSampler                            sampler
		  m_vkImageView,                                  // VkImageView                          imageView
		  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL // VkImageLayout                        imageLayout
		}
	  }
	};

	VKHelper::UpdateDescriptorSets(m_pIntegration->device(), { image_descriptor_update }, { buffer_descriptor_update }, {}, {});

	VkPipelineLayoutCreateInfo plci = {};
	plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plci.pushConstantRangeCount = push_constant_ranges.size();
	plci.pPushConstantRanges = push_constant_ranges.data();
	plci.setLayoutCount = 1;
	plci.pSetLayouts = &m_vkDescriptorSetLayout;
	
	result = vkCreatePipelineLayout(m_pIntegration->device(), &plci, nullptr, &m_vkPipelineLaout);
	CError::CheckError<VkResult>(result);


	//Create Pipeline
	
	VkPipelineInputAssemblyStateCreateInfo ia = {};
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	ia.primitiveRestartEnable = false;

	VkPipelineRasterizationStateCreateInfo ra = {};
	ra.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	ra.depthClampEnable			= false;
	ra.rasterizerDiscardEnable	= false;
	ra.polygonMode				= VK_POLYGON_MODE_FILL;
	ra.cullMode					= VK_CULL_MODE_BACK_BIT;
	ra.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	ra.depthBiasEnable			= false;
	ra.depthBiasConstantFactor	= 0.0f; // Optional
	ra.depthBiasClamp			= 0.0f; // Optional
	ra.depthBiasSlopeFactor		= 0.0f; // Optional
	ra.lineWidth				= 1.0f;


	std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates = {
		{
			false,                          // VkBool32                 blendEnable
			VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcColorBlendFactor
			VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstColorBlendFactor
			VK_BLEND_OP_ADD,                // VkBlendOp                colorBlendOp
			VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcAlphaBlendFactor
			VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstAlphaBlendFactor
			VK_BLEND_OP_ADD,                // VkBlendOp                alphaBlendOp
			VK_COLOR_COMPONENT_R_BIT |      // VkColorComponentFlags    colorWriteMask
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT
		}
	};

	VkPipelineColorBlendStateCreateInfo colblend = {};
	colblend.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colblend.logicOpEnable		= false;
	colblend.logicOp			= VK_LOGIC_OP_COPY;
	colblend.attachmentCount	= attachmentBlendStates.size();
	colblend.pAttachments		= attachmentBlendStates.data();
	colblend.blendConstants[0]	= 1.0f;
	colblend.blendConstants[1]	= 1.0f;
	colblend.blendConstants[2]	= 1.0f;
	colblend.blendConstants[3]	= 1.0f;

	VkPipelineMultisampleStateCreateInfo mul = {};
	mul.sType				  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	mul.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	mul.sampleShadingEnable   = false;
	mul.minSampleShading	  = 0.0f;
	mul.pSampleMask			  = nullptr;
	mul.alphaToCoverageEnable = false;
	mul.alphaToOneEnable	  = false;

	ViewportInfo viewport_infos = {
	{
		{                   // VkViewport     Viewport
			0.0f,               // float          x
			0.0f,               // float          y
			512.0f,             // float          width
			512.0f,             // float          height
			0.0f,               // float          minDepth
			1.0f                // float          maxDepth
		},
	},
	{
		{                   // VkRect2D       Scissor
			{						// VkOffset2D     offset
				0,                  // int32_t        x
				0                   // int32_t        y
			},
			{                   // VkExtent2D     extent
				512,                // uint32_t       width
				512                 // uint32_t       height
			}
		}
	}
	};
	uint32_t viewport_count = static_cast<uint32_t>(viewport_infos.Viewports.size());
	uint32_t scissor_count = static_cast<uint32_t>(viewport_infos.Scissors.size());
	VkPipelineViewportStateCreateInfo vp = {
	  VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,    // VkStructureType                      sType
	  nullptr,                                                  // const void                         * pNext
	  0,                                                        // VkPipelineViewportStateCreateFlags   flags
	  viewport_count,                                           // uint32_t                             viewportCount
	  viewport_infos.Viewports.data(),                          // const VkViewport                   * pViewports
	  scissor_count,                                            // uint32_t                             scissorCount
	  viewport_infos.Scissors.data()                            // const VkRect2D                     * pScissors
	};

	//VkPipelineDepthStencilStateCreateInfo ds = {};
	//ds.sType				 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	//ds.depthTestEnable		 = VK_TRUE;
	//ds.depthWriteEnable		 = VK_TRUE;
	//ds.depthCompareOp		 = VK_COMPARE_OP_LESS_OR_EQUAL;
	//ds.depthBoundsTestEnable = false;
	//ds.minDepthBounds		 = 0.0f;
	//ds.maxDepthBounds		 = 1.0f;
	//ds.stencilTestEnable	 = false;
	//ds.front				 = {};
	//ds.back					 = {};

	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_TRUE;
	ds.depthWriteEnable = VK_TRUE;
	ds.depthCompareOp = VK_COMPARE_OP_LESS;
	ds.depthBoundsTestEnable = VK_FALSE;
	ds.stencilTestEnable = VK_FALSE;
	ds.minDepthBounds = 0.0f; // Optional
	ds.maxDepthBounds = 1.0f; // Optional
	ds.stencilTestEnable = VK_FALSE;
	ds.front = {}; // Optional
	ds.back = {}; // Optional

	std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType			  = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = dynamic_states.size();
	dyn.pDynamicStates    = dynamic_states.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType			   = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pInputAssemblyState = &ia;
	pipelineCreateInfo.pRasterizationState = &ra;
	pipelineCreateInfo.pColorBlendState	   = &colblend;
	pipelineCreateInfo.pMultisampleState   = &mul;
	pipelineCreateInfo.pViewportState	   = &vp;
	pipelineCreateInfo.pDepthStencilState  = &ds;
	pipelineCreateInfo.pDynamicState       = &dyn;
	pipelineCreateInfo.stageCount		   = 1;
	pipelineCreateInfo.pStages			   = &shaderStageCreateInfo;
	pipelineCreateInfo.renderPass		   = m_vkRenderPass;
	pipelineCreateInfo.layout			   = m_vkPipelineLaout;
	pipelineCreateInfo.pVertexInputState   = &vertInputStateCreateInfo;

	result = vkCreateGraphicsPipelines(m_pIntegration->device(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_vkPipeline);
	CError::CheckError<VkResult>(result);

	return result;
}

VkResult Dodo::Rendering::CShadowMapping::UpdateUniformBuffer()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;
	
	ShadowUbo ubo = {};
	
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	Math::Vector3f lightPos(1.0f);
	lightPos.x = cos(glm::radians(time * 360.0f)) * 40.0f;
	lightPos.y = -50.0f + sin(glm::radians(time * 360.0f)) * 20.0f;
	lightPos.z = 25.0f + sin(glm::radians(time * 360.0f)) * 5.0f;


	ubo.light_view_matrix = glm::mat4();
	ubo.scene_view_matrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
	ubo.perspective_matrix = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 96.0f);
	ubo.perspective_matrix[1][1] *= -1;

	glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 96.0f);
	glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
	glm::mat4 depthModelMatrix = glm::mat4(1.0f);

	Math::Matrix4x4 depthMVP = Math::Matrix4x4();
	depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

	void* data;
	vkMapMemory(m_pIntegration->device(), m_vkUniformBufferMemory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(m_pIntegration->device(), m_vkUniformBufferMemory);

	return result;
}

VkResult Dodo::Rendering::CShadowMapping::RecordCommandBuffer(VkCommandBuffer _cb)
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;
	
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.2f, 0.2f, 0.2f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo rbi = {};
	rbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rbi.clearValueCount = clearValues.size();
	rbi.pClearValues = clearValues.data();
	rbi.framebuffer = m_vkFramebuffer;
	rbi.renderPass = m_vkRenderPass;
	rbi.renderArea = { { 0, 0, }, { 512, 512 } };

	vkCmdBeginRenderPass(_cb, &rbi, VK_SUBPASS_CONTENTS_INLINE);

	std::vector<Entity::CEntity*> ents = Entity::CEntityHandler::GetEntities();
	std::vector<std::shared_ptr<Components::CMesh>> meshes;

	for (auto ent : ents)
	{
		std::shared_ptr<Components::CMesh> mesh = ent->GetComponent<Components::CMesh>();
		if (mesh != nullptr)
		{
			meshes.push_back(mesh);
		}
	}


	vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
	vkCmdBindDescriptorSets(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLaout, 0, 1, &m_vkDescriptorSet, 0, nullptr);

	VkDeviceSize offsets[1] = { 0 };
	for (int j = 0; j < meshes.size(); j++)
	{
		vkCmdBindVertexBuffers(_cb, 0, 1, &meshes[j]->m_dataBuffers.vertexBuffer, offsets);
		vkCmdBindIndexBuffer(_cb, meshes[j]->m_dataBuffers.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(_cb, meshes[j]->m_indices.size(), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(_cb);

	return result;
}
