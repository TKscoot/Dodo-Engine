#include "dodopch.h"
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
	CreateCommandPool();
	CreateDepthResources();
	CreateFramebuffers();
	CreateTextureImages();
	CreateTextureImageViews();
	CreateTextureSampler();
	CreateVertexBuffers();
	CreateIndexBuffers();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();

	// GUI
	m_pGui = std::make_shared<Rendering::GUI>(m_pIntegration, &m_pCamera->cameraPos, &m_pCamera->cameraFront, m_pCamera->camSpeed);
	m_pGui->CreateRenderPass(m_vkSwapChainImageFormat, FindDepthFormat());
	m_pGui->CreateFramebuffers(m_vkSwapChainImageViews, m_vkDepthImageView, m_vkSwapChainExtent);
	m_pGui->Initialize(m_vkRenderPass, m_vkSwapChainExtent.width, m_vkSwapChainExtent.height);

	CreateCommandBuffers();
	CreateSyncObjects();



	return DODO_OK;
}

DodoError Dodo::Rendering::CRenderer::DrawFrame(double deltaTime)
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

	// Update uniform buffers here!!
	UpdateUniformBuffer();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	std::vector<VkSemaphore> waitSemaphores	  = { m_sSyncObjects.imageAvailableSemaphores[m_iCurrentFrame] };
	std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	std::vector<VkCommandBuffer> commandBuffers;
	if (m_bDrawGui)
	{
		commandBuffers = {m_vkCommandBuffers.at(imageIndex), m_pGui->m_vkCommandBuffers.at(imageIndex)};

	}
	else
	{
		commandBuffers = { m_vkCommandBuffers.at(imageIndex) };
	}

	submitInfo.waitSemaphoreCount	= 1;
	submitInfo.pWaitSemaphores		= waitSemaphores.data();
	submitInfo.pWaitDstStageMask	= waitStages.data();
	submitInfo.commandBufferCount   = commandBuffers.size();
	submitInfo.pCommandBuffers      = commandBuffers.data();
	
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

	m_fFrameTimeCounter += deltaTime;

	if (m_bDrawGui)
	{
		if (vkWaitForFences(m_pIntegration->device(),
			1,
			&m_sSyncObjects.inFlightFences[m_iCurrentFrame],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max()) == VK_SUCCESS)
		{
			//UpdateCommandBuffers();

			if (m_fFrameTimeCounter >= 1.0f)
			{
				m_pGui->NewFrame(deltaTime, true);
				m_fFrameTimeCounter = 0.0f;
			}
			else
			{
				m_pGui->NewFrame(deltaTime, false);
			}

			if (m_pGui->UpdateBuffers())
			{
				m_pGui->DrawFrame(m_vkSwapChainExtent, m_vkRenderPass, m_vkSwapChainFramebuffers, deltaTime);
			}
		}
	}

	m_iCurrentFrame = (m_iCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	result = DODO_OK;

	return result;
}

DodoError Dodo::Rendering::CRenderer::Finalize()
{
	CleanupSwapChain();
	m_pGui->Finalize();

	for (auto &mat : m_pMaterials)
	{
		mat->Finalize();
	}

	vkDestroyDescriptorSetLayout(m_pIntegration->device(), m_vkDescriptorSetLayout, nullptr);

	for (auto &mesh : m_pMeshes)
	{
		vkDestroyBuffer(m_pIntegration->device(), mesh->m_dataBuffers.indexBuffer, nullptr);
		vkFreeMemory(m_pIntegration->device(), mesh->m_dataBuffers.indexBufferMemory, nullptr);

		vkDestroyBuffer(m_pIntegration->device(), mesh->m_dataBuffers.vertexBuffer, nullptr);
		vkFreeMemory(m_pIntegration->device(), mesh->m_dataBuffers.vertexBufferMemory, nullptr);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(m_pIntegration->device(), m_sSyncObjects.renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_pIntegration->device(), m_sSyncObjects.imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_pIntegration->device(), m_sSyncObjects.inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(m_pIntegration->device(), m_vkCommandPool, nullptr);

	m_pIntegration->Finalize();

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

VkImageView Dodo::Rendering::CRenderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(m_pIntegration->device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

VkResult Dodo::Rendering::CRenderer::CreateTextureImage(Components::CMaterial::Texture &_texture)
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkDeviceSize imageSize = _texture.texWidth * _texture.texHeight * 4;


	result = CreateBuffer(imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(m_pIntegration->device(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, _texture.pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_pIntegration->device(), stagingBufferMemory);


	stbi_image_free(_texture.pixels);

	result = CreateImage(
		_texture.texWidth,
		_texture.texHeight,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_texture.textureData.textureImage,
		_texture.textureData.textureImageMemory);

	result = TransitionImageLayout(
		_texture.textureData.textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	result = CopyBufferToImage(
		stagingBuffer,
		_texture.textureData.textureImage,
		static_cast<uint32_t>(_texture.texWidth),
		static_cast<uint32_t>(_texture.texHeight));
	result = TransitionImageLayout(
		_texture.textureData.textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(m_pIntegration->device(), stagingBuffer, nullptr);
	vkFreeMemory(m_pIntegration->device(), stagingBufferMemory, nullptr);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateImageViews()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	m_vkSwapChainImageViews.resize(m_vkSwapChainImages.size());

	for (size_t i = 0; i < m_vkSwapChainImages.size(); i++)
	{
		m_vkSwapChainImageViews[i] = CreateImageView(
													 m_vkSwapChainImages[i], 
													 m_vkSwapChainImageFormat, 
													 VK_IMAGE_ASPECT_COLOR_BIT);
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
	colorAttachment.finalLayout	   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format		   = FindDepthFormat();
	depthAttachment.samples		   = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout	   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout	  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass	 = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass	 = 0;
	dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask	 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType		   = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments	   = attachments.data();
	renderPassInfo.subpassCount	   = 1;
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
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	//VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	//layoutInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//layoutInfo.bindingCount				= 1;
	//layoutInfo.pBindings				= &uboLayoutBinding;

	VkDescriptorSetLayoutBinding albedoSamplerLayoutBinding = {};
	albedoSamplerLayoutBinding.binding = 1;
	albedoSamplerLayoutBinding.descriptorCount = 1;
	albedoSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	albedoSamplerLayoutBinding.pImmutableSamplers = nullptr;
	albedoSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding normalSamplerLayoutBinding = {};
	normalSamplerLayoutBinding.binding = 2;
	normalSamplerLayoutBinding.descriptorCount = 1;
	normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
	normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding metallicSamplerLayoutBinding = {};
	metallicSamplerLayoutBinding.binding = 3;
	metallicSamplerLayoutBinding.descriptorCount = 1;
	metallicSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	metallicSamplerLayoutBinding.pImmutableSamplers = nullptr;
	metallicSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding roughnessSamplerLayoutBinding = {};
	roughnessSamplerLayoutBinding.binding = 4;
	roughnessSamplerLayoutBinding.descriptorCount = 1;
	roughnessSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	roughnessSamplerLayoutBinding.pImmutableSamplers = nullptr;
	roughnessSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 5> bindings = { 
		uboLayoutBinding, 
		albedoSamplerLayoutBinding, 
		normalSamplerLayoutBinding, 
		metallicSamplerLayoutBinding, 
		roughnessSamplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	result = vkCreateDescriptorSetLayout(m_pIntegration->device(), &layoutInfo, nullptr, &m_vkDescriptorSetLayout);
	CError::CheckError<VkResult>(result);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateGraphicsPipeline()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	//for (auto material : m_pMaterials)
	//{
	m_pMaterials[0]->CreateShaders();

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = m_pMaterials[0]->shaders().vertShaderStage.module;
	vertShaderStageInfo.pName = "main";
	shaderStages.push_back(vertShaderStageInfo);

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = m_pMaterials[0]->shaders().fragShaderStage.module;
	fragShaderStageInfo.pName = "main";
	shaderStages.push_back(fragShaderStageInfo);
	//}

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
	rasterizer.frontFace			   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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


	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType				   = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable	   = VK_TRUE;
	depthStencil.depthWriteEnable	   = VK_TRUE;
	depthStencil.depthCompareOp		   = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable     = VK_FALSE;
	depthStencil.minDepthBounds		   = 0.0f; // Optional
	depthStencil.maxDepthBounds		   = 1.0f; // Optional
	depthStencil.stencilTestEnable     = VK_FALSE;
	depthStencil.front				   = {}; // Optional
	depthStencil.back				   = {}; // Optional

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

	std::vector<VkPushConstantRange> pushConstantRanges = {
	vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::vec3), 0),
	vks::initializers::pushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(CMaterial::PushConsts), sizeof(Vector3f)),
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType		  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts	  = &m_vkDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 2;
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	result = vkCreatePipelineLayout(m_pIntegration->device(), &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout);
	CError::CheckError<VkResult>(result);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType			     = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount			 = 2;
	pipelineInfo.pStages			 = shaderStages.data();
	pipelineInfo.pVertexInputState   = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState		 = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState   = &multisampling;
	pipelineInfo.pDepthStencilState  = &depthStencil;
	pipelineInfo.pColorBlendState    = &colorBlending;
	pipelineInfo.pDynamicState		 = nullptr;
	pipelineInfo.layout				 = m_vkPipelineLayout;
	pipelineInfo.renderPass			 = m_vkRenderPass;
	pipelineInfo.subpass			 = 0;
	pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex   = -1;
	pipelineInfo.flags               = VK_DYNAMIC_STATE_SCISSOR | VK_DYNAMIC_STATE_VIEWPORT;


	// on swap recreation shader modules problems
	result = vkCreateGraphicsPipelines(m_pIntegration->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkGraphicsPipeline);
	CError::CheckError<VkResult>(result);
	
	for (auto &stage : shaderStages)
	{
		vkDestroyShaderModule(m_pIntegration->device(), stage.module, nullptr);
	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateFramebuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	m_vkSwapChainFramebuffers.resize(m_vkSwapChainImageViews.size());

	for (size_t i = 0; i < m_vkSwapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
			m_vkSwapChainImageViews[i],
			m_vkDepthImageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= m_vkRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments	= attachments.data();
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
	poolInfo.flags			  = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	result = vkCreateCommandPool(m_pIntegration->device(), &poolInfo, nullptr, &m_vkCommandPool);
	CError::CheckError<VkResult>(result);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateTextureImages()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	for (auto &mat : m_pMaterials)
	{
		CMaterial::Textures* textures = &mat->textures();

		result = CreateTextureImage(textures->albedo   );
		CError::CheckError<VkResult>(result);
		result = CreateTextureImage(textures->normal   );
		CError::CheckError<VkResult>(result);
		result = CreateTextureImage(textures->metallic );
		CError::CheckError<VkResult>(result);
		result = CreateTextureImage(textures->roughness);
		CError::CheckError<VkResult>(result);

	}
	
	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateTextureImageViews()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	for (auto &mat : m_pMaterials)
	{
		CMaterial::Textures* textures = &mat->textures();

		textures->albedo.   textureData.textureImageView    = CreateImageView(textures->albedo.textureData.textureImage,    VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
		textures->normal.   textureData.textureImageView    = CreateImageView(textures->normal.textureData.textureImage,    VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
		textures->metallic. textureData.textureImageView    = CreateImageView(textures->metallic.textureData.textureImage,  VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
		textures->roughness.textureData.textureImageView    = CreateImageView(textures->roughness.textureData.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateTextureSampler()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter				= VK_FILTER_LINEAR;
	samplerInfo.minFilter				= VK_FILTER_LINEAR;
	samplerInfo.addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable		= VK_TRUE;
	samplerInfo.maxAnisotropy			= 16;
	samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable			= VK_FALSE;
	samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias				= 0.0f;
	samplerInfo.minLod					= 0.0f;
	samplerInfo.maxLod					= 0.0f;

	for (auto &mat : m_pMaterials)
	{
		CMaterial::Textures* texures = &mat->textures();

		result = vkCreateSampler(m_pIntegration->device(), &samplerInfo, nullptr, &texures->albedo.textureData.textureSampler);
		result = vkCreateSampler(m_pIntegration->device(), &samplerInfo, nullptr, &texures->normal.textureData.textureSampler);
		result = vkCreateSampler(m_pIntegration->device(), &samplerInfo, nullptr, &texures->metallic.textureData.textureSampler);
		result = vkCreateSampler(m_pIntegration->device(), &samplerInfo, nullptr, &texures->roughness.textureData.textureSampler);
		CError::CheckError<VkResult>(result);
	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateDepthResources()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkFormat depthFormat = FindDepthFormat();

		result = CreateImage(
			m_vkSwapChainExtent.width,
			m_vkSwapChainExtent.height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_vkDepthImage,
			m_vkDepthImageMemory);

		m_vkDepthImageView = CreateImageView(m_vkDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		TransitionImageLayout(m_vkDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);


	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateVertexBuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	for (const std::shared_ptr<Components::CMesh> mesh : m_pMeshes)
	{
		const std::vector<Vertex> verts = mesh->vertices();
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
			mesh->m_dataBuffers.vertexBuffer, mesh->m_dataBuffers.vertexBufferMemory);

		// copying staging buffer to vertex buffer
		CopyBuffer(stagingBuffer, mesh->m_dataBuffers.vertexBuffer, bufferSize);

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

	for (const std::shared_ptr<Components::CMesh> mesh : m_pMeshes)
	{
		VkDeviceSize bufferSize = sizeof(mesh->m_indices[0]) * mesh->m_indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_pIntegration->device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, mesh->m_indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_pIntegration->device(), stagingBufferMemory);

		CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mesh->m_dataBuffers.indexBuffer, mesh->m_dataBuffers.indexBufferMemory);

		CopyBuffer(stagingBuffer, mesh->m_dataBuffers.indexBuffer, bufferSize);

		vkDestroyBuffer(m_pIntegration->device(), stagingBuffer, nullptr);
		vkFreeMemory(m_pIntegration->device(), stagingBufferMemory, nullptr);

		m_matDataBuffers.push_back(mesh->m_dataBuffers);
	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateUniformBuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	m_vkUniformBuffers.resize(m_pTransforms.size());
	m_vkUniformBuffersMemory.resize(m_pTransforms.size());

	for (size_t i = 0; i < m_pTransforms.size(); i++)
	{
		VkDeviceSize bufferSize = sizeof(CMaterial::UniformBufferObject);
	
		result = CreateBuffer(
					bufferSize,
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_vkUniformBuffers[i],
					m_vkUniformBuffersMemory[i]);
		CError::CheckError<VkResult>(result);
	}


	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateCommandBuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	m_vkCommandBuffers.resize(m_vkSwapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_vkCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_vkCommandBuffers.size();

	result = vkAllocateCommandBuffers(m_pIntegration->device(), &allocInfo, m_vkCommandBuffers.data());
	CError::CheckError<VkResult>(result);

	//GUI
	m_pGui->CreateCommandBuffer(m_vkSwapChainFramebuffers, m_vkCommandPool);
	m_pGui->NewFrame(deltaTime(), true);
	m_pGui->UpdateBuffers();
	m_pGui->DrawFrame(m_vkSwapChainExtent, m_vkRenderPass, m_vkSwapChainFramebuffers);

	
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
	
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.2f, 0.2f, 0.2f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
	
		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();
	
		vkCmdBeginRenderPass(m_vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	
		vkCmdBindPipeline(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkGraphicsPipeline);
		
		//std::vector<VkBuffer> vertexBuffers = {};
		VkBuffer *vertexBuffers = new VkBuffer[m_matDataBuffers.size()];
		VkDeviceSize offsets[] = {0};
		for (int j = 0; j < m_pMeshes.size(); j++)
		{
			vertexBuffers[j] = m_matDataBuffers[j].vertexBuffer;
	
			vkCmdBindVertexBuffers(m_vkCommandBuffers[i], 0, 1, &m_matDataBuffers[j].vertexBuffer, offsets);	// evtl buggy bei mehreren vertex buffern
			vkCmdBindIndexBuffer(m_vkCommandBuffers[i], m_pMeshes[j]->m_dataBuffers.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(m_vkCommandBuffers[i], 
									VK_PIPELINE_BIND_POINT_GRAPHICS, 
									m_vkPipelineLayout, 
									0, 
									1, 
									&m_vkDescriptorSets[j], 
									0, 
									nullptr);

			Vector3f pos = m_pMeshes[j]->entity->GetComponent<Components::CTransform>()->getPosition();
			vkCmdPushConstants(m_vkCommandBuffers[i], m_vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Math::Vector3f), &pos);
			float f = 0.5f;
			//m_pMeshes[0]->entity->GetComponent<Components::CMaterial>()->setPushConstants(1.0f, 0.0f, 1.0f, 1.0f, 1.0f);
			//m_pMeshes[1]->entity->GetComponent<Components::CMaterial>()->setPushConstants(0.0f, 0.8f, 1.0f, 1.0f, 1.0f);

			CMaterial::PushConsts pushConsts = m_pMeshes[j]->entity->GetComponent<Components::CMaterial>()->pushConstants();
			//pushConsts.roughness = m_pGui->uiSettings.roughness;
			//pushConsts.metallic = m_pGui->uiSettings.roughness;
			vkCmdPushConstants(m_vkCommandBuffers[i], m_vkPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Vector3f), sizeof(CMaterial::PushConsts), &pushConsts);

			vkCmdDrawIndexed(m_vkCommandBuffers[i], m_pMeshes[j]->m_indices.size(), 1, 0, 0, 0);
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

VkResult Dodo::Rendering::CRenderer::CreateDescriptorPool()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	std::array<VkDescriptorPoolSize, 5> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_vkSwapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(m_vkSwapChainImages.size());
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(m_vkSwapChainImages.size());
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = static_cast<uint32_t>(m_vkSwapChainImages.size());
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[4].descriptorCount = static_cast<uint32_t>(m_vkSwapChainImages.size());
	
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(m_vkSwapChainImages.size());

	result = vkCreateDescriptorPool(m_pIntegration->device(), &poolInfo, nullptr, &m_vkDescriptorPool);
	CError::CheckError<VkResult>(result);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CreateDescriptorSets()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	std::vector<VkDescriptorSetLayout> layouts(m_vkSwapChainImages.size(), m_vkDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType				 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool	 = m_vkDescriptorPool;
	allocInfo.descriptorSetCount = m_vkSwapChainImages.size();
	allocInfo.pSetLayouts		 = layouts.data();

	m_vkDescriptorSets.resize(m_pMeshes.size());
	result = vkAllocateDescriptorSets(m_pIntegration->device(), &allocInfo, m_vkDescriptorSets.data());
	CError::CheckError<VkResult>(result);

	for (size_t i = 0; i < m_pMaterials.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = m_vkUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range  = sizeof(CMaterial::UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		m_pMaterials[i]->textures().albedo.textureData.imageInfo.imageLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_pMaterials[i]->textures().normal.textureData.imageInfo.imageLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_pMaterials[i]->textures().metallic.textureData.imageInfo.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_pMaterials[i]->textures().roughness.textureData.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		m_pMaterials[i]->textures().albedo.textureData.imageInfo.imageView    = m_pMaterials[i]->textures().albedo.textureData.textureImageView;
		m_pMaterials[i]->textures().normal.textureData.imageInfo.imageView    = m_pMaterials[i]->textures().normal.textureData.textureImageView;
		m_pMaterials[i]->textures().metallic.textureData.imageInfo.imageView  = m_pMaterials[i]->textures().metallic.textureData.textureImageView;
		m_pMaterials[i]->textures().roughness.textureData.imageInfo.imageView = m_pMaterials[i]->textures().roughness.textureData.textureImageView;

		m_pMaterials[i]->textures().albedo.textureData.imageInfo.sampler    = m_pMaterials[i]->textures().albedo.textureData.textureSampler;
		m_pMaterials[i]->textures().normal.textureData.imageInfo.sampler    = m_pMaterials[i]->textures().normal.textureData.textureSampler;
		m_pMaterials[i]->textures().metallic.textureData.imageInfo.sampler  = m_pMaterials[i]->textures().metallic.textureData.textureSampler;
		m_pMaterials[i]->textures().roughness.textureData.imageInfo.sampler = m_pMaterials[i]->textures().roughness.textureData.textureSampler;

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			vks::initializers::writeDescriptorSet(m_vkDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo),
			vks::initializers::writeDescriptorSet(m_vkDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &m_pMaterials[i]->textures().albedo.textureData.imageInfo),
			vks::initializers::writeDescriptorSet(m_vkDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &m_pMaterials[i]->textures().normal.textureData.imageInfo),
			vks::initializers::writeDescriptorSet(m_vkDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &m_pMaterials[i]->textures().metallic.textureData.imageInfo),
			vks::initializers::writeDescriptorSet(m_vkDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &m_pMaterials[i]->textures().roughness.textureData.imageInfo)

		};


		vkUpdateDescriptorSets(
			m_pIntegration->device(), 
			static_cast<uint32_t>(writeDescriptorSets.size()),
			writeDescriptorSets.data(), 0,
			nullptr);


	}

	return result;
}

VkResult Dodo::Rendering::CRenderer::UpdateCommandBuffers()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	for (size_t i = 0; i < m_vkCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		result = vkBeginCommandBuffer(m_vkCommandBuffers[i], &beginInfo);
		CError::CheckError<VkResult>(result);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_vkRenderPass;
		renderPassInfo.framebuffer = m_vkSwapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_vkSwapChainExtent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.2f, 0.2f, 0.2f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkGraphicsPipeline);

		if (m_pGui->uiSettings.displayModels)
		{
			VkBuffer *vertexBuffers = new VkBuffer[m_matDataBuffers.size()];
			VkDeviceSize offsets[1] = { 0 };
			for (int j = 0; j < m_pMeshes.size(); j++)
			{
				vertexBuffers[j] = m_matDataBuffers[j].vertexBuffer;

				vkCmdBindVertexBuffers(m_vkCommandBuffers[i], 0, 1, &m_matDataBuffers[j].vertexBuffer, offsets);	// evtl buggy bei mehreren vertex buffern
				vkCmdBindIndexBuffer(m_vkCommandBuffers[i], m_pMeshes[j]->m_dataBuffers.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdBindDescriptorSets(m_vkCommandBuffers[i],
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_vkPipelineLayout,
					0,
					1,
					&m_vkDescriptorSets[j],
					0,
					nullptr);

				Vector3f pos = m_pMeshes[j]->entity->GetComponent<Components::CTransform>()->getPosition();
				vkCmdPushConstants(m_vkCommandBuffers[i], m_vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Math::Vector3f), &pos);
				float f = 0.5f;
				//m_pMeshes[0]->entity->GetComponent<Components::CMaterial>()->setPushConstants(1.0f, 0.0f, 1.0f, 1.0f, 1.0f);
				//m_pMeshes[1]->entity->GetComponent<Components::CMaterial>()->setPushConstants(0.0f, 0.8f, 1.0f, 1.0f, 1.0f);

				CMaterial::PushConsts pushConsts = m_pMeshes[j]->entity->GetComponent<Components::CMaterial>()->pushConstants();
				pushConsts.roughness = m_pGui->uiSettings.roughness;
				pushConsts.metallic = m_pGui->uiSettings.metallic;
				vkCmdPushConstants(m_vkCommandBuffers[i], m_vkPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Vector3f), sizeof(CMaterial::PushConsts), &pushConsts);


				vkCmdDrawIndexed(m_vkCommandBuffers[i], m_pMeshes[j]->m_indices.size(), 1, 0, 0, 0);
			}
		}

		vkCmdEndRenderPass(m_vkCommandBuffers[i]);

		result = vkEndCommandBuffer(m_vkCommandBuffers[i]);
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

VkResult Dodo::Rendering::CRenderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory)
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType     = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width  = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth  = 1;
	imageInfo.mipLevels		= 1;
	imageInfo.arrayLayers	= 1;
	imageInfo.format		= format;
	imageInfo.tiling		= tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage			= usage;
	imageInfo.samples		= VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags			= 0; // Optional

	result = vkCreateImage(m_pIntegration->device(), &imageInfo, nullptr, &image);
	CError::CheckError<VkResult>(result);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_pIntegration->device(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = m_pIntegration->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	result = vkAllocateMemory(m_pIntegration->device(), &allocInfo, nullptr, &imageMemory);
	CError::CheckError<VkResult>(result);


	vkBindImageMemory(
		m_pIntegration->device(),
		image,
		imageMemory,
		0);
	return result;
}

VkResult Dodo::Rendering::CRenderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkResult result = VK_SUCCESS;

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout			= oldLayout;
	barrier.newLayout			= newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image				= image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(commandBuffer);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height)
{
	VkResult result = VK_SUCCESS;

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset					   = 0;
	region.bufferRowLength				   = 0;
	region.bufferImageHeight			   = 0;
	region.imageSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel	   = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount	   = 1;
	region.imageOffset					   = { 0, 0, 0 };
	region.imageExtent					   = {
											 	_width,
											 	_height,
											 	1
											 };

	vkCmdCopyBufferToImage(
		commandBuffer,
		_buffer,
		_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);

	EndSingleTimeCommands(commandBuffer);

	return result;
}

VkResult Dodo::Rendering::CRenderer::CopyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size		 = _size;

	vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);

	return result;
}

VkCommandBuffer Dodo::Rendering::CRenderer::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool		 = m_vkCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_pIntegration->device(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Dodo::Rendering::CRenderer::EndSingleTimeCommands(VkCommandBuffer _buf)
{
	vkEndCommandBuffer(_buf);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_buf;

	vkQueueSubmit(m_pIntegration->queues().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_pIntegration->queues().graphicsQueue);

	vkFreeCommandBuffers(m_pIntegration->device(), m_vkCommandPool, 1, &_buf);
}

VkResult Dodo::Rendering::CRenderer::UpdateUniformBuffer()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
	m_dDeltaTime = time;

	for (int i = 0; i < m_pTransforms.size(); i++)
	{
		CMaterial::UniformBufferObject ubo = {};
		if (m_pTransforms[i] != nullptr)
		{
			//m_pTransforms[i]->setPositionY(time * 0.005f);
		ubo.model	   = m_pTransforms[i]->getComposed();

		}
		else
		{
		ubo.model	   = glm::rotate(glm::mat4(1.0f), time *  glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		}
		ubo.camPos     = m_pCamera->cameraPos;
		ubo.View	   = m_pCamera->getViewMatrix();
		ubo.projection = m_pCamera->getProjectionMatrix();
		ubo.projection[1][1] *= -1;

		void* data;
		vkMapMemory(m_pIntegration->device(), m_vkUniformBuffersMemory[i], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(m_pIntegration->device(), m_vkUniformBuffersMemory[i]);
	}


	return VK_SUCCESS;
}

VkResult Dodo::Rendering::CRenderer::CleanupSwapChain()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	vkDestroyImageView(m_pIntegration->device(), m_vkDepthImageView, nullptr);
	vkDestroyImage(m_pIntegration->device(), m_vkDepthImage, nullptr);
	vkFreeMemory(m_pIntegration->device(), m_vkDepthImageMemory, nullptr);

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

	for (size_t i = 0; i < m_pTransforms.size(); i++)
	{
		vkDestroyBuffer(m_pIntegration->device(), m_vkUniformBuffers[i], nullptr);
		vkFreeMemory(m_pIntegration->device(), m_vkUniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(m_pIntegration->device(), m_vkDescriptorPool, nullptr);

	return result;
}

VkResult Dodo::Rendering::CRenderer::RecreateSwapChain()
{
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_pWindow->GetWindow(), &width, &height);
		glfwWaitEvents();
	}

	result = vkDeviceWaitIdle(m_pIntegration->device());
	CError::CheckError<VkResult>(result);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateDepthResources();
	CreateFramebuffers();
	m_pGui->CreateFramebuffers(m_vkSwapChainImageViews, m_vkDepthImageView, m_vkSwapChainExtent);
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
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

VkFormat Dodo::Rendering::CRenderer::FindSupportedFormat(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features)
{
	for (VkFormat format : _candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_pIntegration->physicalDevice(), format, &props);

		if (_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features)
		{
			return format;
		}
		else if (_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & _features) == _features)
		{
			return format;
		}
	}

	CLog::Error("Couldnt find supported format");
	return VK_FORMAT_END_RANGE;
}
