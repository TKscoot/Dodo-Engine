#pragma once
#include "dodopch.h"
#include "VKIntegration.h"
#include <fstream>

namespace Dodo
{
	namespace Rendering
	{
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

		};
	}
}
