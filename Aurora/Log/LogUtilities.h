#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "../Utilities/Console.h"

namespace Aurora
{
	enum class LogType
	{
		Info,			// Standard debugging information.
		Warning,		// Action is passable but unadvised.
		Error,			// This shouldn't be happening, but we will let it proceed if workable.
		Critical		// This must not happen or something really bad will happen. Perhaps an apocalypse? Immediately ends the program execution.
	};

	enum class LogLayer
	{
		Initialization,
		Engine,
		Graphics,
		Physics,
		Event,
		Serialization,
		ECS,
		Editor,
		Assert,
		Audio,
		Scripting
	};

	inline void SetLogLayerColor(LogLayer logLayer)
	{
		switch (logLayer)
		{
			case LogLayer::Initialization:
				Console::Shade::SetColor(Console::Color::Color_Blue);
				std::cout << "[Initialization] ";
				break;

			case LogLayer::Event:
				Console::Shade::SetColor(Console::Color::Color_LightPurple);
				std::cout << "[Event] ";
				break;

			case LogLayer::Graphics:
				Console::Shade::SetColor(Console::Color::Color_LightBlue);
				std::cout << "[Graphics] ";
				break;

			case LogLayer::Engine:
				Console::Shade::SetColor(Console::Color::Color_LightAqua);
				std::cout << "[Engine] ";
				break;

			case LogLayer::Physics:
				Console::Shade::SetColor(Console::Color::Color_BrightWhite);
				std::cout << "[Physics] ";
				break;

			case LogLayer::Serialization:
				Console::Shade::SetColor(Console::Color::Color_Purple);
				std::cout << "[Serialization] ";
				break;

			case LogLayer::ECS:
				Console::Shade::SetColor(Console::Color::Color_BrightWhite);
				std::cout << "[ECS] ";
				break;

			case LogLayer::Editor:
				Console::Shade::SetColor(Console::Color::Color_Yellow);
				std::cout << "[Editor] ";
				break;

			case LogLayer::Assert:
				Console::Shade::SetColor(Console::Color::Color_Red);
				std::cout << "[Assert] ";
				break;

			case LogLayer::Audio:
				Console::Shade::SetColor(Console::Color::Color_Grey);
				std::cout << "[Audio] ";
				break;

			case LogLayer::Scripting:
				Console::Shade::SetColor(Console::Color::Color_Purple);
				std::cout << "[Scripting] ";
				break;
		}

		Console::Shade::Reset();

	}

	struct LogPackage
	{
		LogPackage() {}
		LogPackage(const std::string& text, const std::pair<LogType, Console::Color>& logType) : m_Text(text), m_LogType(logType)
		{
			// Retrieve time.
			std::chrono::time_point currentTimePoint = std::chrono::system_clock::now();
			time_t currentTime = std::chrono::system_clock::to_time_t(currentTimePoint);
			std::tm timeBuffer;
			localtime_s(&timeBuffer, &currentTime);

			std::stringstream timeString;
			timeString << std::put_time(&timeBuffer, "[%H:%M:%S]");

			m_Timestamp = timeString.str();
		}

		LogPackage(const std::string& text, LogType logType, const std::string& logSource) : m_Text(text), m_LogType(std::pair(logType, Console::Color::Color_Black)), m_LogSource(logSource)
		{
			// Retrieve time.
			std::chrono::time_point currentTimePoint = std::chrono::system_clock::now();
			time_t currentTime = std::chrono::system_clock::to_time_t(currentTimePoint);
			std::tm timeBuffer;
			localtime_s(&timeBuffer, &currentTime);

			std::stringstream timeString;
			timeString << std::put_time(&timeBuffer, "[%H:%M:%S]");

			m_Timestamp = timeString.str();
		}

		std::string EditorConsoleText()
		{
			return m_Timestamp + " " + m_Text;
		}

		std::string m_Text;
		std::string m_Timestamp = "[19:50:45]";
		std::string m_LogSource = "Toolbar::CreateConsole() (at Assets/Scripts/PlacingSystem.cpp:41)";
		std::pair<LogType, Console::Color> m_LogType = { LogType::Info, Console::Color::Color_Green };
	};
}