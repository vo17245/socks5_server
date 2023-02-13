#include "Log.h"



Log& Log::Get()
{
	static Log log;
	return log;
}
Log::Log()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	m_Logger = spdlog::stdout_color_mt("Server");
	m_Logger->set_level(spdlog::level::trace);
}
Log::~Log()
{
}





