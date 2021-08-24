#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <vector>
// #include "ILogger.h"
#include "LogUtilities.h"

/* Personal Notes:

	- Log to file.
	- Further information in log such as line number and timestamp.
	- Hook up to Editor.
*/

namespace Aurora
{
	// Macros for different forms of logging information.
	#define AURORA_INFO(level, text, ...)	     { Aurora::Log::WriteInfoLog(static_cast<Aurora::LogLayer>(level), std::string(__FUNCTION__)    + ": " + std::string(text), __VA_ARGS__); }
	#define AURORA_WARNING(level, text, ...)     { Aurora::Log::WriteWarningLog(static_cast<Aurora::LogLayer>(level), std::string(__FUNCTION__) + ": " + std::string(text), __VA_ARGS__); }
	#define AURORA_ERROR(level, text, ...)	     { Aurora::Log::WriteErrorLog(static_cast<Aurora::LogLayer>(level), std::string(__FUNCTION__)   + ": " + std::string(text), __VA_ARGS__); } 
	#define AURORA_CRITICAL(level, text, ...)    { Aurora::Log::WriteCriticalLog(static_cast<Aurora::LogLayer>(level), std::string(__FUNCTION__)   + ": " + std::string(text), __VA_ARGS__); }

	#define AURORA_ASSERT_ERROR(text, ...)	     { Aurora::Log::WriteErrorLog(LogLayer::Engine, std::string(__FUNCTION__)   + ": " + std::string(text), __VA_ARGS__); } 
	//Standard Errors
	#define AURORA_ERROR_GENERIC_FAILURE()	    AURORA_ERROR("Failed.");
	#define AURORA_ERROR_INVALID_PARAMETER()	AURORA_ASSERT_ERROR("Invalid Parameter.");
	#define AURORA_ERROR_INVALID_INTERNALS()	AURORA_ERROR("Invalid Internals.");

	class Log
	{
		friend class ILogger;
		#define LOG_TYPE std::pair<LogType, Console::Color>

	public:
		Log() = default;

		// Set a logger to be used.
		// static void SetLogger(const std::weak_ptr<ILogger>& logger) { m_Logger = logger; }

		// Alphabetical
		static void WriteLog(LogLayer logLayer, const char* logMessage, const LOG_TYPE logType);

		static void WriteInfoLog(LogLayer logLayer, const char* logMessage, ...);
		static void WriteWarningLog(LogLayer logLayer, const char* logMessage, ...);
		static void WriteErrorLog(LogLayer logLayer, const char* logMessage, ...);
		static void WriteCriticalLog(LogLayer logLayer, const char* logMessage, ...);

		static void WriteLog(const std::string& logMessage, const LOG_TYPE logType);

		static void WriteInfoLog(LogLayer logLayer, const std::string logMessage, ...);
		static void WriteWarningLog(LogLayer logLayer, const std::string logMessage, ...);
		static void WriteErrorLog(LogLayer logLayer, const std::string logMessage, ...);
		static void WriteCriticalLog(LogLayer logLayer, const std::string logMessage, ...);

		// Numeric
		template<typename T, typename std::enable_if<
			std::is_same<T, int>::value ||
			std::is_same<T, long>::value ||
			std::is_same<T, long long>::value ||
			std::is_same<T, unsigned>::value ||
			std::is_same<T, unsigned long>::value ||
			std::is_same<T, unsigned long long>::value ||
			std::is_same<T, float>::value ||
			std::is_same<T, double>::value ||
			std::is_same<T, long double>::value>::type>
			static void WriteLog(T value, const std::string& logSource, LogType logType)
		{
			WriteLog(std::to_string(value), logSource, logType);
		}

	private:
		static void FlushBuffer();
		static void LogString(const char* logMessage, const std::string& logSource, LogType logType);

		// Console
		static void LogToConsole(LogLayer logLayer, const char* logMessage, LOG_TYPE logType);

	private:
		// static std::weak_ptr<ILogger> m_Logger;
		static bool m_ConsoleLoggingEnabled;

		static std::mutex m_MutexLog;
		static std::vector<LogPackage> m_LogPackages;
	};
}