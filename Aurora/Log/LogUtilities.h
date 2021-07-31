#pragma once
#include <string>
#include <Windows.h>

namespace Aurora
{
	const WORD WhiteConsoleTextColor = 15;
	const WORD BoldConsoleTextColor = FOREGROUND_INTENSITY;
	const WORD BlueConsoleTextColor = 11;
	const WORD GreenConsoleTextColor = FOREGROUND_GREEN;
	const WORD YellowConsoleTextColor = 14;
	const WORD RedConsoleTextColor = 12;
	const WORD CriticalConsoleTextColor = 12;

	enum class LogType
	{
		Info,
		Warning,
		Error,
	};

	enum class LogLayer
	{
		Engine,
		Graphics,
		Physics
	};

	struct LogPackage
	{
		LogPackage() {}
		LogPackage(const std::string& text, LogType logType) : m_Text(text), m_LogLevel(logType)
		{

		}

		LogPackage(const std::string& text, const std::string& logSource, LogType logType) : m_Text(text), m_LogSource(logSource), m_LogLevel(logType)
		{

		}

		std::string EditorConsoleText()
		{
			return m_Timestamp + " " + m_Text;
		}

		std::string m_Text;
		std::string m_Timestamp = "[19:50:45]";
		std::string m_LogSource = "Toolbar::CreateConsole() (at Assets/Scripts/PlacingSystem.cpp:41)";
		LogType m_LogLevel = LogType::Info;
	};
}