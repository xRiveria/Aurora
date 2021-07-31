#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include "LogUtilities.h"

/* Personal Notes:

	- Log to file.
	- Further information in log such as line number and timestamp.
	- Hook up to Editor.
*/

namespace Aurora
{
	// Macros for different forms of logging information.
	#define AURORA_INFO(text, ...)				 { Aurora::Log::WriteInfoLog(std::string(__FUNCTION__)    + ": " + std::string(text), __VA_ARGS__); }

	#define AURORA_WARNING(text, ...)     { Aurora::Log::WriteWarningLog(std::string(__FUNCTION__) + ": " + std::string(text), __VA_ARGS__); }
	#define AURORA_ERROR(text, ...)       { Aurora::Log::WriteErrorLog(std::string(__FUNCTION__)   + ": " + std::string(text), __VA_ARGS__); }
	
	//Standard Errors
	#define AURORA_ERROR_GENERIC_FAILURE()	    AURORA_ERROR("Failed.");
	#define AURORA_ERROR_INVALID_PARAMETER()	AURORA_ERROR("Invalid Parameter.");
	#define AURORA_ERROR_INVALID_INTERNALS()	AURORA_ERROR("Invalid Internals.");

	class Log
	{
	public:
		Log() = default;

		//Alphabetical
		static void WriteLog(const char* logMessage, const LogType logType);
		static void WriteInfoLog(LogLayer logLayer, const char* logMessage, ...);
		static void WriteWarningLog(const char* logMessage, ...);
		static void WriteErrorLog(const char* logMessage, ...);

		static void WriteLog(const std::string& logMessage, const LogType logType);
		static void WriteInfoLog(const std::string logMessage, ...);
		static void WriteWarningLog(const std::string logMessage, ...);
		static void WriteErrorLog(const std::string logMessage, ...);

		//Numeric
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
		static void LogToConsole(const char* logMessage, LogType logType);
		static void SetConsoleTextColor(LogType logType);
		static void RestoreConsoleAttributes();

	private:
		static bool m_ConsoleLoggingEnabled;

		static std::mutex m_MutexLog;
		static std::vector<LogPackage> m_LogPackages;

		// Console
		static HANDLE m_OutHandle;
		static CONSOLE_SCREEN_BUFFER_INFO m_DefaultConsoleState;
	};
}