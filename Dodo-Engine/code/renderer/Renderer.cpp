#include "Renderer.h"

using namespace Dodo::Environment;

bool Dodo::Rendering::CRenderer::m_bFramebufferResized = false;

DodoError Dodo::Rendering::CRenderer::Initialize(std::shared_ptr<VKIntegration> _integration, std::shared_ptr<CWindow> _window)
{
	m_pIntegration = _integration;
	m_pWindow	   = _window;

	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffers();
	CreateIndexBuffers();
	CreateUniformBuffers();
	CreateCommandBuffers();
	CreateSyncObjects();

	return DODO_OK;
}

DodoError Dodo::Rendering::CRenderer::DrawFrame()
{
	DodoError result = DODO_INITIALIZATION_FAILED;
	VkResult vkResult = VK_ERROR_INITIALIZATION_FAILED;

	uint32_t imageIndex = 0;

	vkResult = vkWaitForFences(m_pIntegration->device(),
							   1,
							   &m_sSyncObjects.inFlightFences[m_iCurrentFrame],
							   VK_TRUE,
							   std::numeric_limits<uint64_t>::max());
	CError::CheckError<VkResult>(vkResult);

	vkResult = vkResetFences(m_pIntegration->device(), 1, &m_sSyncObjects.inFlightFences[m_iCurrentFrame]);
	CError::CheckError<VkResult>(vkResult);

	vkResult = vkAcquireNextImageKHR(m_pIntegration->device(), 
									 m_vkSwapChain, 
									 std::numeric_limits<uint64_t>::max(), 
									 m_sSyncObjects.imageAvailableSemaphores[m_iCurrentFrame], 
									 VK_NULL_HANDLE, 
									 &imageIndex);
	
	// check if swapchain needs to be recreated
	if (vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR || m_bFramebufferResized)
	{
		m_bFramebufferResized = false;
		RecreateSwapChain();
	}
	else
	{
		CError::CheckError<VkResult>(vkResult);
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	std::vector<VkSemaphore> waitSemaphores	  = { m_sSyncObjects.imageAvailableSemaphores[m_iCurrentFrame] };
	std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount	= 1;
	submitInfo.pWaitSemaphores		= waitSemaphores.data();
	submitInfo.pWaitDstStageMask	= waitStages.data();
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= &m_vkCommandBuffers.at(imageIndex);
	
	std::vector<VkSemaphore> signalSemaphores	= { m_sSyncObjects.renderFinishedSemaphores[m_iCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores	= signalSemaphores.data();

	vkResetFences(m_pIntegration->device(), 1, &m_sSyncObjects.inFlightFences[m_iCurrentFrame]);
	
	vkResult = vkQueueSubmit(m_pIntegration->queues().graphicsQueue, 1, &submitInfo, m_sSyncObjects.inFlightFences[m_iCurrentFrame]);
	CError::CheckError<VkResult>(vkResult);

	VkPresentInfoKHR presentInfo   = {};
	presentInfo.sType			   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores    = signalSemaphores.data();

	presentInfo.swapchainCount     = 1;
	presentInfo.pSwapchains		   = &m_vkSwapChain;
	presentInfo.pImageIndices	   = &imageIndex;
	presentInfo.pResults		   = nullptr;

	vkResult = vkQueuePresentKHR(m_pIntegration->queues().presentQueue, &presentInfo);
	CError::CheckError<VkResult>(vkResult);

	m_iCurrentFrame = (m_iCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	result = DODO_OK;

	return result;
}

DodoError Dodo::Rendering::CRenderer::Finalize()
{
	CleanupSwapChain();
	return DODO_OK;
}

VkResult Dodo::Rendering::CRenderer::CreateSwapChain()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	SwapChainSupportDetails swapChainDetails = QuerySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainDetails.formats);
	VkPresentModeKHR   presentMode   = ChooseSwapPresentMode(swapChainDetails.presentModes);
	VkExtent2D         extent		 = ChooseSwapExtent(swapChainDetails.capabilities);

	// Number of images in the swap chain
	uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;
	if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
	{
		imageCount = swapChainDetails.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface			= m_pIntegration->surface();
	createInfo.minImageCount	= imageCount;
	createInfo.imageFormat		= surfaceFormat.format;
	createInfo.imageColorSpace	= surfaceFormat.colorSpace;
	createInfo.imageExtent		= extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//uint32_t queueFamilyIndices[] = { 
	//	(uint32_t)m_pIntegration->queueFamilies().graphicsQueueFamilyIndices, 
	//	(uint32_t)m_pIntegration->queueFamilies().presentQueueFamilyIndices };

	std::vector<uint32_t> queueFamilyIndices(2);
	queueFamilyIndices = {
		(uint32_t)m_pIntegration->queueFamilies().graphicsQueueFamilyIndices,
		(uint32_t)m_pIntegration->queueFamilies().presentQueueFamilyIndices };

	if (m_pIntegration->queueFamilies().graphicsQueueFamilyIndices != m_pIntegration->queueFamilies().presentQueueFamilyIndices)
	{
		createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
		createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform   = swapChainDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode    = presentMode;
	createInfo.clipped        = VK_TRUE;

	result = vkCreateSwapchainKHR(m_pIntegration->device(), &createInfo, nullptr, &m_vkSwapChain);
	CError::CheckError<VkResult>(result);

	result = vkGetSwapchainImagesKHR(m_pIntegration->device(), m_vkSwapChain, &imageCount, nullptr);
	m_vkSwapChainImages.resize(imageCount);
	result = vkGetSwapchainImagesKHR(m_pIntegration->device(), m_vkSwapChain, &imageCount, m_vkSwapChainImages.data());

	m_vkSwapChainImageFormat = surfaceFormat.format;
	m_vkSwapChainExtent = extent;

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateImageViews()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	m_vkSwapChainImageViews.resize(m_vkSwapChainImages.size());

	for (size_t i = 0; i < m_vkSwapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType						   = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image						   = m_vkSwapChainImages[i];
		createInfo.viewType						   = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format						   = m_vkSwapChainImageFormat;
		createInfo.components.r					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel   = 0;
		createInfo.subresourceRange.levelCount	   = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount     = 1;

		result = vkCreateImageView(m_pIntegration->device(), &createInfo, nullptr, &m_vkSwapChainImageViews[i]);
		CError::CheckError<VkResult>(result);
	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateRenderPass()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format		   = m_vkSwapChainImageFormat;
	colorAttachment.samples		   = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment  = 0;
	colorAttachmentRef.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount   = 1;
	subpass.pColorAttachments	   = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass		   = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass          = 0;
	dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask	   = 0;
	dependency.dstStageMask		   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask	   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType		   = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments	   = &colorAttachment;
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies   = &dependency;

	result = vkCreateRenderPass(m_pIntegration->device(), &renderPassInfo, nullptr, &m_vkRenderPass);
	CError::CheckError<VkResult>(result);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateDescriptorSetLayout()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;
	
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding			= 0;
	uboLayoutBinding.descriptorCount	= 1;
	uboLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;	// image sampling related
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount				= 1;
	layoutInfo.pBindings				= &uboLayoutBinding;

	result = vkCreateDescriptorSetLayout(m_pIntegration->device(), &layoutInfo, nullptr, &m_vkDescriptorSetLayout);
	CError::CheckError<VkResult>(result);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateGraphicsPipeline()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	for (auto material : m_pMaterials)
	{

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = material->shaders().vertShaderStage.module;
		vertShaderStageInfo.pName = "main";
		shaderStages.push_back(vertShaderStageInfo);

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = material->shaders().fragShaderStage.module;
		fragShaderStageInfo.pName = "main";
		shaderStages.push_back(fragShaderStageInfo);
	}

	auto bindingDescriptions   = CMaterial::GetBindingDescription();
	auto attributeDescriptions = CMaterial::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType						    = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount   = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions		= &bindingDescriptions;
	vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType					 = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology				 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x		    = 0.0f;
	viewport.y		    = 0.0f;
	viewport.width	    = (float)m_vkSwapChainExtent.width;
	viewport.height	    = (float)m_vkSwapChainExtent.height;
	viewport.minDepth   = 0.0f;
	viewport.maxDepth   = 1.0f;

	VkRect2D scissor = {};
	scissor.offset   = { 0, 0 };
	scissor.extent   = m_vkSwapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports	= &viewport;
	viewportState.scissorCount	= 1;
	viewportState.pScissors		= &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType				   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable		   = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode			   = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth			   = 1.0f;
	rasterizer.cullMode				   = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace			   = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable		   = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp		   = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor	   = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable	= VK_FALSE;
	multisampling.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading		= 1.0f; // Optional
	multisampling.pSampleMask			= nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable		= VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask		 = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable		 = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp		 = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp		 = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable		= VK_FALSE;
	colorBlending.logicOp			= VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount	= 1;
	colorBlending.pAttachments		= &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType		  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts	  = &m_vkDescriptorSetLayout;

	result = vkCreatePipelineLayout(m_pIntegration->device(), &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout);
	CError::CheckError<VkResult>(result);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = m_vkPipelineLayout;
	pipelineInfo.renderPass = m_vkRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;


	// on swap recreation shader modules problems
	result = vkCreateGraphicsPipelines(m_pIntegration->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkGraphicsPipeline);
	CError::CheckError<VkResult>(result);

	//for (auto &material : m_pMaterials)
	//{
	//	vkDestroyShaderModule(m_pIntegration->device(), material->shaders().fragShaderStage.module, nullptr);
	//	vkDestroyShaderModule(m_pIntegration->device(), material->shaders().vertShaderStage.module, nullptr);
	//}
	
	

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateFramebuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	m_vkSwapChainFramebuffers.resize(m_vkSwapChainImageViews.size());

	for (size_t i = 0; i < m_vkSwapChainFramebuffers.size(); i++)
	{
		VkImageView attachments[] = {
			m_vkSwapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= m_vkRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments	= attachments;
		framebufferInfo.width			= m_vkSwapChainExtent.width;
		framebufferInfo.height			= m_vkSwapChainExtent.height;
		framebufferInfo.layers			= 1;

		result = vkCreateFramebuffer(m_pIntegration->device(), &framebufferInfo, nullptr, &m_vkSwapChainFramebuffers[i]);
		CError::CheckError<VkResult>(result);
	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateCommandPool()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType			  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_pIntegration->queueFamilies().graphicsQueueFamilyIndices;
	poolInfo.flags			  = 0;

	result = vkCreateCommandPool(m_pIntegration->device(), &poolInfo, nullptr, &m_vkCommandPool);
	CError::CheckError<VkResult>(result);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateVertexBuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	for (const std::shared_ptr<Rendering::CMaterial> material : m_pMaterials)
	{
		const std::vector<Vertex> verts = material->vertices();
		VkDeviceSize bufferSize = sizeof(verts[0]) * verts.size();

		// creating staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		result = CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, stagingBufferMemory);

		// copying vert data to staging buffer
		void* data;
		vkMapMemory(m_pIntegration->device(), stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, verts.data(), (size_t)bufferSize);
		vkUnmapMemory(m_pIntegration->device(), stagingBufferMemory);

		// creating vertex Buffer
		CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			material->m_dataBuffers.vertexBuffer, material->m_dataBuffers.vertexBufferMemory);

		// copying staging buffer to vertex buffer
		CopyBuffer(stagingBuffer, material->m_dataBuffers.vertexBuffer, bufferSize);

		vkDestroyBuffer(m_pIntegration->device(), stagingBuffer, nullptr);
		vkFreeMemory(m_pIntegration->device(), stagingBufferMemory, nullptr);

		// push vertex buffer to array for further use (buffer binding, etc.)
		//m_matDataBuffers.push_back(buffer);
	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateIndexBuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	for (const std::shared_ptr<Rendering::CMaterial> material : m_pMaterials)
	{
		VkDeviceSize bufferSize = sizeof(material->indices[0]) * material->indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_pIntegration->device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, material->indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_pIntegration->device(), stagingBufferMemory);

		CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			material->m_dataBuffers.indexBuffer, material->m_dataBuffers.indexBufferMemory);

		CopyBuffer(stagingBuffer, material->m_dataBuffers.indexBuffer, bufferSize);

		vkDestroyBuffer(m_pIntegration->device(), stagingBuffer, nullptr);
		vkFreeMemory(m_pIntegration->device(), stagingBufferMemory, nullptr);

		// now push all material dataBuffers to array for further use (buffer binding, etc.) ---- TODO: evtl. MaterialHandler klasse schreiben
		//m_matDataBuffers.push_back(material->m_dataBuffers);
	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateUniformBuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	for (const std::shared_ptr<Rendering::CMaterial> material : m_pMaterials)
	{
		VkDeviceSize bufferSize = sizeof(CMaterial::UniformBufferObject);

		result = CreateBuffer(
					bufferSize,
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					material->m_dataBuffers.uniformBuffer, 
					material->m_dataBuffers.uniformBufferMemory);
		CError::CheckError<VkResult>(result);

		m_matDataBuffers.push_back(material->m_dataBuffers);
	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateCommandBuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	m_vkCommandBuffers.resize(m_vkSwapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool		 = m_vkCommandPool;
	allocInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_vkCommandBuffers.size();

	result = vkAllocateCommandBuffers(m_pIntegration->device(), &allocInfo, m_vkCommandBuffers.data());
	CError::CheckError<VkResult>(result);

	for (size_t i = 0; i < m_vkCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType			   = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags			   = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		result = vkBeginCommandBuffer(m_vkCommandBuffers[i], &beginInfo);
		CError::CheckError<VkResult>(result);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_vkRenderPass;
		renderPassInfo.framebuffer = m_vkSwapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_vkSwapChainExtent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(m_vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkGraphicsPipeline);

		//std::vector<VkBuffer> vertexBuffers = {};
		VkBuffer *vertexBuffers = new VkBuffer[m_matDataBuffers.size()];
		VkDeviceSize offsets[] = {0};
		for (int j = 0; j < m_pMaterials.size(); j++)
		{
			vertexBuffers[j] = m_matDataBuffers[j].vertexBuffer;

			vkCmdBindVertexBuffers(m_vkCommandBuffers[i], 0, 1, &m_matDataBuffers[j].vertexBuffer, offsets);	// evtl buggy bei mehreren vertex buffern
			vkCmdBindIndexBuffer(m_vkCommandBuffers[i], m_matDataBuffers[j].indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(m_vkCommandBuffers[i], m_pMaterials[j]->indices.size(), 1, 0, 0, 0);

			//vkCmdDraw(m_vkCommandBuffers[i], 3, 1, 0, 0);	// only drawing one triangle even 2 vert buffers are bound
		}

		vkCmdEndRenderPass(m_vkCommandBuffers[i]);

		result = vkEndCommandBuffer(m_vkCommandBuffers[i]);
		CError::CheckError<VkResult>(result);

	}


	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateSyncObjects()
{
	m_sSyncObjects.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_sSyncObjects.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_sSyncObjects.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		result = vkCreateSemaphore(m_pIntegration->device(), &semaphoreInfo, nullptr, &m_sSyncObjects.imageAvailableSemaphores[i]);
		CError::CheckError<VkResult>(result);
		result = vkCreateSemaphore(m_pIntegration->device(), &semaphoreInfo, nullptr, &m_sSyncObjects.renderFinishedSemaphores[i]);
		CError::CheckError<VkResult>(result);
		result = vkCreateFence(m_pIntegration->device(), &fenceInfo, nullptr, &m_sSyncObjects.inFlightFences[i]);
		CError::CheckError<VkResult>(result);
	}


	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer &_buffer, VkDeviceMemory &_bufferMemory)
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType	   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size		   = _size;
	bufferInfo.usage	   = _usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	result = vkCreateBuffer(m_pIntegration->device(), &bufferInfo, nullptr, &_buffer);
	CError::CheckError<VkResult>(result);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_pIntegration->device(), _buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType			  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = m_pIntegration->FindMemoryType(
												memRequirements.memoryTypeBits, _properties);

	result = vkAllocateMemory(m_pIntegration->device(), &allocInfo, nullptr, &_bufferMemory);
	CError::CheckError<VkResult>(result);

	vkBindBufferMemory(m_pIntegration->device(), _buffer, _bufferMemory, 0);


	return result;
}

VkResult Dodo::Rendering::CRenderer::CopyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_vkCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(m_pIntegration->device(), &allocInfo, &commandBuffer);
	CError::CheckError<VkResult>(result);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size		 = _size;

	vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers	  = &commandBuffer;

	vkQueueSubmit(m_pIntegration->queues().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE); // evtl fence erstellen, aber eigentlich unnötig
	vkQueueWaitIdle(m_pIntegration->queues().graphicsQueue);

	vkFreeCommandBuffers(m_pIntegration->device(), m_vkCommandPool, 1, &commandBuffer);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CleanupSwapChain()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	for (size_t i = 0; i < m_vkSwapChainFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(m_pIntegration->device(), m_vkSwapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(m_pIntegration->device(), m_vkCommandPool, m_vkCommandBuffers.size(), m_vkCommandBuffers.data());

	vkDestroyPipeline(m_pIntegration->device(), m_vkGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_pIntegration->device(), m_vkPipelineLayout, nullptr);
	vkDestroyRenderPass(m_pIntegration->device(), m_vkRenderPass, nullptr);
	
	for (size_t i = 0; i < m_vkSwapChainImageViews.size(); i++)
	{
		vkDestroyImageView(m_pIntegration->device(), m_vkSwapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_pIntegration->device(), m_vkSwapChain, nullptr);

	return result;
}

VkResult Dodo::Rendering::CRenderer::RecreateSwapChain()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	result = vkDeviceWaitIdle(m_pIntegration->device());
	CError::CheckError<VkResult>(result);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateUniformBuffers();
	CreateCommandBuffers();



	return result;
}

Dodo::Rendering::CRenderer::SwapChainSupportDetails Dodo::Rendering::CRenderer::QuerySwapChainSupport()
{
	SwapChainSupportDetails details = {};
	// Get capabilities
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pIntegration->physicalDevice(),
																m_pIntegration->surface(), 
																&details.capabilities);
	CError::CheckError<VkResult>(result);

	// Get formats
	uint32_t formatCount = 0;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_pIntegration->physicalDevice(),
												  m_pIntegration->surface(), 
												  &formatCount, 
												  nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_pIntegration->physicalDevice(),
													  m_pIntegration->surface(),
													  &formatCount,
													  details.formats.data());

	}
	CError::CheckError<VkResult>(result);

	// Get present modes
	uint32_t presentModeCount = 0;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pIntegration->physicalDevice(), 
													   m_pIntegration->surface(), 
													   &presentModeCount, 
													   nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pIntegration->physicalDevice(),
														   m_pIntegration->surface(),
														   &presentModeCount,
														   details.presentModes.data());
	}
	CError::CheckError<VkResult>(result);

	return details;
}

VkSurfaceFormatKHR Dodo::Rendering::CRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats)
{
	if (_availableFormats.size() == 1 && _availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : _availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return _availableFormats[0];
}

VkPresentModeKHR Dodo::Rendering::CRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> _availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : _availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = availablePresentMode;
		}
	}
	return bestMode;
}

VkExtent2D Dodo::Rendering::CRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR & _capabilities)
{
	if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() /*UINT32_MAX*/)
	{
		return _capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(m_pWindow->GetWindow(), &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = glm::max(_capabilities.minImageExtent.width, glm::min(_capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = glm::max(_capabilities.minImageExtent.height, glm::min(_capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}