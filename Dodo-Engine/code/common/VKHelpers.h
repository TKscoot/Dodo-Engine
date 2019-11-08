#pragma once
#include "dodopch.h"
#include "VKIntegration.h"
#include "renderer/VulkanInitializers.h"
#include "renderer/VulkanBuffer.h"
#include <fstream>

namespace Dodo
{
	namespace Rendering
	{

#define DEFAULT_FENCE_TIMEOUT 100000000000
		class VKHelper
		{
		public:
			static inline VkShaderModule CreateShaderModule(std::shared_ptr<VKIntegration> _integration, std::string _filename)
			{
				std::ifstream file(_filename, std::ios::ate | std::ios::binary);

				if (!file.is_open())
				{
					Environment::CLog::Error("Failed to open shader file!");
				}

				size_t fileSize = (size_t)file.tellg();
				std::vector<char> buffer(fileSize);

				file.seekg(0);
				file.read(buffer.data(), fileSize);
				file.close();

				VkShaderModuleCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = buffer.size();
				createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

				VkShaderModule shaderModule = VK_NULL_HANDLE;

				VkResult result = vkCreateShaderModule(_integration->device(), &createInfo, nullptr, &shaderModule);
				CError::CheckError<VkResult>(result);

				return shaderModule;
			}

			static inline VkCommandBuffer BeginSingleTimeCommands(std::shared_ptr<VKIntegration> _integration, VkCommandPool _commandPool)
			{
				VkCommandBufferAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandPool = _commandPool;
				allocInfo.commandBufferCount = 1;

				VkCommandBuffer commandBuffer;
				vkAllocateCommandBuffers(_integration->device(), &allocInfo, &commandBuffer);

				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

				vkBeginCommandBuffer(commandBuffer, &beginInfo);

				return commandBuffer;
			}

			static inline void EndSingleTimeCommands(std::shared_ptr<VKIntegration> _integration, VkCommandPool _commandPool, VkCommandBuffer _buf)
			{
				vkEndCommandBuffer(_buf);

				VkSubmitInfo submitInfo = {};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &_buf;

				vkQueueSubmit(_integration->queues().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(_integration->queues().graphicsQueue);

				vkFreeCommandBuffers(_integration->device(), _commandPool, 1, &_buf);
			}

			static inline VkResult CreateBuffer(std::shared_ptr<VKIntegration> _integration, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer &_buffer, VkDeviceMemory &_bufferMemory)
			{
				VkResult result = VK_ERROR_INITIALIZATION_FAILED;

				VkBufferCreateInfo bufferInfo = {};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = _size;
				bufferInfo.usage = _usage;
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				result = vkCreateBuffer(_integration->device(), &bufferInfo, nullptr, &_buffer);
				CError::CheckError<VkResult>(result);

				VkMemoryRequirements memRequirements;
				vkGetBufferMemoryRequirements(_integration->device(), _buffer, &memRequirements);

				VkMemoryAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = memRequirements.size;
				allocInfo.memoryTypeIndex = _integration->FindMemoryType(
					memRequirements.memoryTypeBits, _properties);

				result = vkAllocateMemory(_integration->device(), &allocInfo, nullptr, &_bufferMemory);
				CError::CheckError<VkResult>(result);

				vkBindBufferMemory(_integration->device(), _buffer, _bufferMemory, 0);


				return result;
			}

			static inline VkResult CopyBuffer(std::shared_ptr<VKIntegration> _integration, VkCommandPool _commandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
			{
				VkResult result = VK_ERROR_INITIALIZATION_FAILED;

				VkCommandBuffer commandBuffer = BeginSingleTimeCommands(_integration, _commandPool);

				VkBufferCopy copyRegion = {};
				copyRegion.srcOffset = 0;
				copyRegion.dstOffset = 0;
				copyRegion.size = _size;

				vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

				EndSingleTimeCommands(_integration, _commandPool, commandBuffer);

				return result;
			}

			static inline void setImageLayout(
				VkCommandBuffer cmdbuffer,
				VkImage image,
				VkImageLayout oldImageLayout,
				VkImageLayout newImageLayout,
				VkImageSubresourceRange subresourceRange,
				VkPipelineStageFlags srcStageMask,
				VkPipelineStageFlags dstStageMask)
			{
				// Create an image barrier object
				VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
				imageMemoryBarrier.oldLayout = oldImageLayout;
				imageMemoryBarrier.newLayout = newImageLayout;
				imageMemoryBarrier.image = image;
				imageMemoryBarrier.subresourceRange = subresourceRange;

				// Source layouts (old)
				// Source access mask controls actions that have to be finished on the old layout
				// before it will be transitioned to the new layout
				switch (oldImageLayout)
				{
				case VK_IMAGE_LAYOUT_UNDEFINED:
					// Image layout is undefined (or does not matter)
					// Only valid as initial layout
					// No flags required, listed only for completeness
					imageMemoryBarrier.srcAccessMask = 0;
					break;

				case VK_IMAGE_LAYOUT_PREINITIALIZED:
					// Image is preinitialized
					// Only valid as initial layout for linear images, preserves memory contents
					// Make sure host writes have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					// Image is a color attachment
					// Make sure any writes to the color buffer have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					// Image is a depth/stencil attachment
					// Make sure any writes to the depth/stencil buffer have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					// Image is a transfer source 
					// Make sure any reads from the image have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					// Image is a transfer destination
					// Make sure any writes to the image have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					// Image is read by a shader
					// Make sure any shader reads from the image have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				default:
					// Other source layouts aren't handled (yet)
					break;
				}

				// Target layouts (new)
				// Destination access mask controls the dependency for the new image layout
				switch (newImageLayout)
				{
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					// Image will be used as a transfer destination
					// Make sure any writes to the image have been finished
					imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					// Image will be used as a transfer source
					// Make sure any reads from the image have been finished
					imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					// Image will be used as a color attachment
					// Make sure any writes to the color buffer have been finished
					imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					// Image layout will be used as a depth/stencil attachment
					// Make sure any writes to depth/stencil buffer have been finished
					imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					// Image will be read in a shader (sampler, input attachment)
					// Make sure any writes to the image have been finished
					if (imageMemoryBarrier.srcAccessMask == 0)
					{
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
					}
					imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				default:
					// Other source layouts aren't handled (yet)
					break;
				}

				// Put barrier inside setup command buffer
				vkCmdPipelineBarrier(
					cmdbuffer,
					srcStageMask,
					dstStageMask,
					0,
					0, nullptr,
					0, nullptr,
					1, &imageMemoryBarrier);
			}

			// Commandbuffer stuff

			static inline VkCommandPool createCommandPool(std::shared_ptr<VKIntegration> integration, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
			{
				VkCommandPoolCreateInfo cmdPoolInfo = {};
				cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
				cmdPoolInfo.flags = createFlags;
				VkCommandPool cmdPool;
				CError::CheckError<VkResult>(vkCreateCommandPool(integration->device(), &cmdPoolInfo, nullptr, &cmdPool));
				return cmdPool;
			}

			static inline VkCommandBuffer createCommandBuffer(std::shared_ptr<VKIntegration> integration, VkCommandBufferLevel level, VkCommandPool commandPool, bool begin = false)
			{
				VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPool, level, 1);

				VkCommandBuffer cmdBuffer;
				CError::CheckError<VkResult>(vkAllocateCommandBuffers(integration->device(), &cmdBufAllocateInfo, &cmdBuffer));

				// If requested, also start recording for the new command buffer
				if (begin)
				{
					VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
					CError::CheckError<VkResult>(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
				}

				return cmdBuffer;
			}

			static inline void flushCommandBuffer(std::shared_ptr<VKIntegration> integration, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool, bool free = true)
			{
				if (commandBuffer == VK_NULL_HANDLE)
				{
					return;
				}

				CError::CheckError<VkResult>(vkEndCommandBuffer(commandBuffer));

				VkSubmitInfo submitInfo = vks::initializers::submitInfo();
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &commandBuffer;

				// Create fence to ensure that the command buffer has finished executing
				VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo(0);
				VkFence fence;
				CError::CheckError<VkResult>(vkCreateFence(integration->device(), &fenceInfo, nullptr, &fence));

				// Submit to the queue
				CError::CheckError<VkResult>(vkQueueSubmit(queue, 1, &submitInfo, fence));
				// Wait for the fence to signal that command buffer has finished executing
				CError::CheckError<VkResult>(vkWaitForFences(integration->device(), 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

				vkDestroyFence(integration->device(), fence, nullptr);

				if (free)
				{
					vkFreeCommandBuffers(integration->device(), commandPool, 1, &commandBuffer);
				}
			}
		};
	}
}
