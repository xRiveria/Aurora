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
			ImGui::MenuItem("New Scene", "Ctrl + N");
			ImGui::Separator();

			if (ImGui::MenuItem("Save Scene", "Ctrl + S"))
			{
				m_EngineContext->GetSubsystem<Aurora::World>()->SerializeScene();
			}

			if (ImGui::MenuItem("Open Scene", "Ctrl + O)"))
			{
				std::optional<std::string> filePath = EditorExtensions::OpenFilePath("Scene File", m_EngineContext);

				if (filePath.has_value())
				{
					AURORA_INFO("%s", filePath.value().c_str());
					m_EngineContext->GetSubsystem<Aurora::World>()->DeserializeScene(filePath.value());
				}
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