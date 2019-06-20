#include "dodopch.h"
#include "Log.h"

std::shared_ptr<spdlog::logger> Dodo::Environment::CLog::m_pConsole = {};

void Dodo::Environment::CLog::CreateLogger(std::string _loggerName)
{
	m_pConsole = spdlog::stdout_color_mt(_loggerName);
}
