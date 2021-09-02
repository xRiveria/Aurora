#include "MenuBar.h"
#include "../Scene/World.h"
#include "../Threading/Threading.h"
#include "../Backend/Utilities/Extensions.h"
#include <optional>
#include "../Utilities/Version.h"
#include "../Core/Settings.h"
#include "../Core/SettingsUtilities.h"

MenuBar::MenuBar(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
	m_WidgetName = "Menu Bar";
	m_IsWindowedWidget = false;
}

void MenuBar::OnTickAlways()
{
	Aurora::AURORA_PROFILE_FUNCTION();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene", "Ctrl + N"))
			{
				NewScene();
			}

			if (ImGui::MenuItem("Save Scene", "Ctrl + S"))
			{
				SaveScene();
			}

			if (ImGui::MenuItem("Load Scene", "Ctrl + L"))
			{
				LoadScene();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Close"))
			{
				m_EngineContext->GetSubsystem<Aurora::WindowContext>()->SetWindowRunState(false);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::Separator();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Assets"))
		{
			ImGui::Separator();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Object"))
		{

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows"))
		{

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("About", nullptr, &m_IsShowingAboutWindow);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	ShowAboutWindow();
}

void MenuBar::OnEvent(Aurora::InputEvent& inputEvent)
{
	Aurora::InputEventDispatcher dispatcher(inputEvent);
	dispatcher.Dispatch<Aurora::KeyPressedEvent>(AURORA_BIND_INPUT_EVENT(MenuBar::OnKeyPressed));
}

void MenuBar::ShowAboutWindow()
{
	if (!m_IsShowingAboutWindow)
	{
		return;
	}

	ImGui::SetNextWindowFocus();
	ImGui::Begin("About", &m_IsShowingAboutWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
	ImGui::Text("Aurora Engine %i.%i", Aurora::GetVersionMajor(), Aurora::GetVersionMinor());
	ImGui::Text("Author: Ryan Tan");
	ImGui::SameLine(ImGui::GetWindowContentRegionWidth());
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 55.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
	if (ImGui::Button("Github"))
	{
		Aurora::FileSystem::OpenDirectoryWindow("https://github.com/xRiveria/Aurora");
	}

	ImGui::Separator();
	ImGui::BeginChildFrame(ImGui::GetID("License"), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 15.5f), ImGuiWindowFlags_NoMove);
	ImGui::Text("MIT License");
	ImGui::Text("Permission is hereby granted, free of charge, to any person obtaining a copy");
	ImGui::Text("of this software and associated documentation files(the \"Software\"), to deal");
	ImGui::Text("in the Software without restriction, including without limitation the rights");
	ImGui::Text("to use, copy, modify, merge, publish, distribute, sublicense, and / or sell");
	ImGui::Text("copies of the Software, and to permit persons to whom the Software is furnished");
	ImGui::Text("to do so, subject to the following conditions :");
	ImGui::Text("The above copyright notice and this permission notice shall be included in");
	ImGui::Text("all copies or substantial portions of the Software.");
	ImGui::EndChildFrame();

	ImGui::Separator();

	ImGui::Text("Third Party Libraries");
	{
		ImGui::Text("Name");
		ImGui::SameLine(220.0f);
		ImGui::Text("Version");
		ImGui::SameLine(340.0f);
		ImGui::Text("URL");

		ImGui::Separator();

		for (const Aurora::ExternalLibrary& externalLibrary : m_EngineContext->GetSubsystem<Aurora::Settings>()->GetThirdPartyLibraries())
		{
			ImGui::BulletText(externalLibrary.m_Name.c_str());
			ImGui::SameLine(220.0f);
			ImGui::Text(externalLibrary.m_Version.c_str());
			ImGui::SameLine(340.0f);
			ImGui::Text(externalLibrary.m_URL.c_str());
		}
	}

	ImGui::End();
}

bool MenuBar::OnKeyPressed(Aurora::KeyPressedEvent& keyPressedEvent)
{
	if (keyPressedEvent.GetRepeatCount() > 0)
	{
		return false;
	}

	bool isControlPressed = m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_LEFT_CONTROL) || m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_RIGHT_CONTROL);
	bool isShiftPressed = m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_LEFT_SHIFT) || m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_RIGHT_SHIFT);

	switch (keyPressedEvent.GetKeyCode())
	{
		case AURORA_KEY_N:
		{
			if (isControlPressed) { NewScene(); }
			break;
		}

		case AURORA_KEY_L:
		{
			if (isControlPressed) { LoadScene(); }
			break;
		}

		case AURORA_KEY_S:
		{
			if (isControlPressed && isShiftPressed) { SaveScene(); }
			break;
		}
	}
}

void MenuBar::SaveScene()
{
	std::optional<std::string> filePath = EditorExtensions::SaveFile("Aurora Scene (*.aurora)\0*.aurora\0", m_EngineContext);

	if (filePath.has_value())
	{
		AURORA_INFO(Aurora::LogLayer::Serialization, "%s", filePath.value().c_str());

		m_EngineContext->GetSubsystem<Aurora::Threading>()->Execute([this, filePath](Aurora::JobInformation jobInformation)
		{
			m_EngineContext->GetSubsystem<Aurora::World>()->SerializeScene(filePath.value());
		});
	}
}

void MenuBar::LoadScene()
{
	std::optional<std::string> filePath = EditorExtensions::OpenFile("Aurora Scene (*.aurora)\0*.aurora\0", m_EngineContext);

	if (filePath.has_value())
	{
		AURORA_INFO(Aurora::LogLayer::Serialization, "%s", filePath.value().c_str());

		m_EngineContext->GetSubsystem<Aurora::Threading>()->Execute([this, filePath](Aurora::JobInformation jobInformation)
		{
			m_EngineContext->GetSubsystem<Aurora::World>()->DeserializeScene(filePath.value());
		});
	}
}

void MenuBar::NewScene()
{
	m_EngineContext->GetSubsystem<Aurora::World>()->New();
}
