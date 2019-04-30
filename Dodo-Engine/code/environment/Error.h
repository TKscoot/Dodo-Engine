#pragma once
#include "common/VKIntegration.h"
#include "Log.h"

namespace Dodo
{
	namespace Environment
	{
		enum DodoError { DODO_OK = 0, DODO_FAILED, DODO_INITIALIZATION_FAILED, DODO_NULL  };
		class CError
		{

		public:

			template<typename type>
			static inline void CheckError(type _err)
			{
				if (typeid(type) == typeid(VkResult))
				{
					if (_err != VK_SUCCESS)
					{
						CLog::Error("VULKAN: ", (int)_err);
						throw("Vulkan error code: ", _err);
					}
				}
				if (typeid(type) == typeid(DodoError))
				{
					if (_err != DODO_OK)
					{
						CLog::Error("DODO: ", (int)_err);
						throw("DODO error code: ", _err);
					}
				}
				else
				{
					if (_err != 0)
					{
						CLog::Error("MISC: ", (int)_err);
						throw("Misc error code: ", _err);
					}
				}
			}


		};
	}
}
