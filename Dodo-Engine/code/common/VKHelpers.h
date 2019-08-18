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

		};
	}
}
