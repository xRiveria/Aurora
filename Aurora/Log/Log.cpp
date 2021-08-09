#include "Aurora.h"

namespace Aurora
{
	std::mutex Log::m_MutexLog;
	std::vector<LogPackage> Log::m_LogPackages;

	bool Log::m_ConsoleLoggingEnabled = true;

	//Everything resolves to this.
	void Log::WriteLog(LogLayer logLayer, const char* logMessage, const LOG_TYPE logType)
	{
		if (!logMessage)
		{
			AURORA_ERROR_INVALID_PARAMETER();
			return;
		}

		std::lock_guard<std::mutex> lockGuard(m_MutexLog);

		std::string logText = std::string(logMessage);

		std::size_t sourceInformationIndex = logText.find("Source: ");
		std::string logTextExtracted = logText.substr(0, sourceInformationIndex);
		std::string logSource = logText.substr(sourceInformationIndex + 8, logText.length());
	
		FlushBuffer();
		
		LogToConsole(logLayer, logTextExtracted.c_str(), logType);
	}

	void Log::LogString(const char* logMessage, const std::string& logSource, LogType logType)
	{
		if (!logMessage)
		{
			AURORA_ERROR_INVALID_PARAMETER();
			return;
		}
	}

	void Log::LogToConsole(LogLayer logLayer, const char* logMessage, LOG_TYPE logType)
	{
		SetLogLayerColor(logLayer);

		Console::Shade::SetColor(logType.second);
		std::cout << logMessage << "\n";
		Console::Shade::Reset();
	}

	void Log::WriteInfoLog(LogLayer logLayer, const char* logMessage, ...)
	{
		char buffer[2048];

		//va_list is a complete object type suitable for holding the information needed by the va_ macros. If a va_list instance is created, passed to another function and
		//used via va_arg in that function, then any subsequent use in the calling function should be preceded by a call to va_end.
		va_list arguments;

		//The va_start macro enables access to the variable arguments following the named argument. va_start should be invoked with an instance to a valid va_list object before any calls.
		va_start(arguments, logMessage);

		//Composes a string with the same text that would be printed if the content was used on printf, but using the elements in the variable argument list identified 
		//by the va_list arg instead of additional function arguments, storing the resulting content as a C-string buffer in a buffer. Internally, the function should 
		//have been initialized by va_start at some point before the call, and is expected to be released by va_end at some point after the call.
		auto w = vsnprintf(buffer, sizeof(buffer), logMessage, arguments);

		//The va_end macro performs cleanup for an object initialized by a call to va_start or va_copy. If there is no corresponding call to va_start or va_copy, 
		//or if va_end is not called before a function that calls va_start or va_copy returns, the behavior is undefined.
		va_end(arguments);

		WriteLog(logLayer, buffer, { LogType::Info, Console::Color::Color_Green });
	}

	void Log::WriteWarningLog(LogLayer logLayer, const char* logMessage, ...)
	{
		char buffer[2048];

		va_list arguments;
		va_start(arguments, logMessage);
		auto w = vsnprintf(buffer, sizeof(buffer), logMessage, arguments);
		va_end(arguments);

		WriteLog(logLayer, buffer, { LogType::Warning, Console::Color::Color_LightYellow });
	}

	void Log::WriteErrorLog(LogLayer logLayer, const char* logMessage, ...)
	{
		char buffer[2048];

		va_list arguments;
		va_start(arguments, logMessage);
		auto w = vsnprintf(buffer, sizeof(buffer), logMessage, arguments);
		va_end(arguments);

		WriteLog(logLayer, buffer, { LogType::Error, Console::Color::Color_LightRed });
	}

	void Log::WriteCriticalLog(LogLayer logLayer, const char* logMessage, ...)
	{
	}

	void Log::WriteLog(const std::string& logMessage, LOG_TYPE logType)
	{
		WriteLog(logMessage.c_str(), logType);
	}

	void Log::WriteInfoLog(LogLayer logLayer, const std::string logMessage, ...)
	{
		char buffer[2048];

		va_list arguments;
		va_start(arguments, logMessage);
		auto w = vsnprintf(buffer, sizeof(buffer), logMessage.c_str(), arguments);
		va_end(arguments);

		WriteLog(logLayer, buffer, { LogType::Info, Console::Color::Color_LightGreen });
	}

	void Log::WriteWarningLog(LogLayer logLayer, const std::string logMessage, ...)
	{
		char buffer[2048];

		va_list arguments;
		va_start(arguments, logMessage);
		auto w = vsnprintf(buffer, sizeof(buffer), logMessage.c_str(), arguments);
		va_end(arguments);

		WriteLog(logLayer, buffer, { LogType::Warning, Console::Color::Color_LightYellow });
	}

	void Log::WriteErrorLog(LogLayer logLayer, const std::string logMessage, ...)
	{
		char buffer[2048];

		va_list arguments;
		va_start(arguments, logMessage);
		auto w = vsnprintf(buffer, sizeof(buffer), logMessage.c_str(), arguments);
		va_end(arguments);

		WriteLog(logLayer, buffer, { LogType::Error, Console::Color::Color_LightRed });
	}

	void Log::WriteCriticalLog(LogLayer logLayer, const std::string logMessage, ...)
	{
		char buffer[2048];

		va_list arguments;
		va_start(arguments, logMessage);
		auto w = vsnprintf(buffer, sizeof(buffer), logMessage.c_str(), arguments);
		va_end(arguments);

		WriteLog(logLayer, buffer, { LogType::Critical, Console::Color::Color_Red });
	}

	void Log::FlushBuffer()
	{
		if (m_LogPackages.empty())
		{
			return;
		}

		//Log everything from memory to the logger implementation.
		for (const LogPackage& logPackage : m_LogPackages)
		{
			LogString(logPackage.m_Text.c_str(), logPackage.m_LogSource, logPackage.m_LogType.first);
		}

		m_LogPackages.clear();
	}
}