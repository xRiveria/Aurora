#include "MenuBar.h"
#include "../Scene/World.h"
#include "../Backend/Utilities/Extensions.h"
#include <optional>

MenuBar::MenuBar(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
	m_WidgetName = "Menu Bar";
	m_IsWindowedWidget = false;
}

void MenuBar::OnTickAlways()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene", "Ctrl + N"))
			{
				m_EngineContext->GetSubsystem<Aurora::World>()->New();
			}

			if (ImGui::MenuItem("Save Scene", "Ctrl + S"))
			{
				std::optional<std::string> filePath = EditorExtensions::SaveFile("Aurora Scene (*.aurora)\0*.aurora\0", m_EngineContext);
				
				if (filePath.has_value())
				{
					AURORA_INFO("%s", filePath.value().c_str());
					m_EngineContext->GetSubsystem<Aurora::World>()->SerializeScene(filePath.value());
				}
			}

			if (ImGui::MenuItem("Open Scene", "Ctrl + O"))
			{
				std::optional<std::string> filePath = EditorExtensions::OpenFile("Aurora Scene (*.aurora)\0*.aurora\0", m_EngineContext);

				if (filePath.has_value())
				{
					AURORA_INFO("%s", filePath.value().c_str());
					m_EngineContext->GetSubsystem<Aurora::World>()->DeserializeScene(filePath.value());
				}
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

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}