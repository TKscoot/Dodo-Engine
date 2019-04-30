#pragma once
#include <spdlog/spdlog.h>
#include <string>
namespace Dodo
{
	namespace Environment
	{
		class CLog
		{
		public:
			static void SetFormat(std::string _formatterString)
			{
				spdlog::set_pattern(_formatterString);
			}

			static void Message(std::string _msg, int _code = 0)
			{
				if (_code != 0)
				{
					_msg += " Return code: " + std::to_string(_code);
				}
				spdlog::info(_msg.c_str());
			}

			static void Warning(std::string _msg, int _code = 0)
			{
				if (_code != 0)
				{
					_msg += " Return code: " + std::to_string(_code);
				}
				spdlog::warn(_msg.c_str());
			}

			static void Error(std::string _msg, int _code = 0)
			{
				if (_code != 0)
				{
					_msg += " Return code: " + std::to_string(_code);
				}
				spdlog::error(_msg.c_str());
			}

			static void Critical(std::string _msg, int _code = 0)
			{
				if (_code != 0)
				{
					_msg += " Return code: " + std::to_string(_code);
				}
				spdlog::critical(_msg.c_str());
			}
		};		
		
	}
}

