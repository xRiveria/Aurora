#pragma once
#include <string>
#include <iostream>
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
		Assert
	};

	inline void SetLogLayerColor(LogLayer logLayer)
	{
		switch (logLayer)
		{
			case LogLayer::Initialization:
				Console::Shade::SetColor(Console::Color::Color_Blue);
				std::cout << "[INITIALIZATION] ";
				break;

			case LogLayer::Event:
				Console::Shade::SetColor(Console::Color::Color_LightPurple);
				std::cout << "[EVENT] ";
				break;

			case LogLayer::Graphics:
				Console::Shade::SetColor(Console::Color::Color_LightBlue);
				std::cout << "[GRAPHICS] ";
				break;

			case LogLayer::Engine:
				Console::Shade::SetColor(Console::Color::Color_LightAqua);
				std::cout << "[ENGINE] ";
				break;

			case LogLayer::Physics:
				Console::Shade::SetColor(Console::Color::Color_BrightWhite);
				std::cout << "[PHYSICS] ";
				break;

			case LogLayer::Serialization:
				Console::Shade::SetColor(Console::Color::Color_Purple);
				std::cout << "[SERIALIZATION] ";
				break;

			case LogLayer::ECS:
				Console::Shade::SetColor(Console::Color::Color_BrightWhite);
				std::cout << "[ECS] ";
				break;

			case LogLayer::Assert:
				Console::Shade::SetColor(Console::Color::Color_Red);
				std::cout << "[ASSERT] ";
				break;
		}

		Console::Shade::Reset();

	}

	struct LogPackage
	{
		LogPackage() {}
		LogPackage(const std::string& text, const std::pair<LogType, Console::Color>& logType) : m_Text(text), m_LogType(logType)
		{

		}

		LogPackage(const std::string& text, const std::string& logSource, const std::pair<LogType, Console::Color>& logType) : m_Text(text), m_LogSource(logSource), m_LogType(logType)
		{

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