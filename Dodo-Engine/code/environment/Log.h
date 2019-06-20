#pragma once
#include "dodopch.h"

namespace Dodo
{
	namespace Environment
	{


		class CLog
		{
		public:
			static void CreateLogger(std::string _loggerName);

			static void SetFormat(std::string _formatterString)
			{
				m_pConsole->set_pattern(_formatterString);
			}

			static void Message(std::string _msg, int _code = 0)
			{
				if (_code != 0)
				{
					_msg += " Return code: " + std::to_string(_code);
				}
				m_pConsole->info(_msg.c_str());
			}

			static void Warning(std::string _msg, int _code = 0)
			{
				if (_code != 0)
				{
					_msg += " Return code: " + std::to_string(_code);
				}
				m_pConsole->warn(_msg.c_str());
			}

			static void Error(std::string _msg, int _code = 0)
			{
				if (_code != 0)
				{
					_msg += " Return code: " + std::to_string(_code);
				}
				m_pConsole->error(_msg.c_str());
			}

			static void Critical(std::string _msg, int _code = 0)
			{
				if (_code != 0)
				{
					_msg += " Return code: " + std::to_string(_code);
				}
				m_pConsole->critical(_msg.c_str());
			}

		private:
			static std::shared_ptr<spdlog::logger> m_pConsole;
		};		
		
	}
}

