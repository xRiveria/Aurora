#include "Toolbar.h"
#include "../Backend/Source/imgui_internal.h"
#include "../Backend/Utilities/Extensions.h"

Toolbar::Toolbar(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
	m_WidgetName = "Toolbar";
	m_IsWindowedWidget = false; //Our toolbar is a permanent fixture above our viewport and below our menu bar.

	m_WidgetFlags = ImGuiWindowFlags_NoCollapse		 |
					ImGuiWindowFlags_NoResize		 |
					ImGuiWindowFlags_NoMove			 |
					ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoScrollbar	 |
					ImGuiWindowFlags_NoTitleBar;

	//Hook up our widgets here that we wish to add to the toolbar.
}

void Toolbar::OnTickAlways()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
	float height = ImGui::GetFrameHeight() + 18.0f;

	const auto DisplayToolbarIcon = [this](const IconType& iconType)
	{
		//ImGui::SameLine(); //All our icons are to be on the same line. 
		if (EditorExtensions::ImageButton(iconType, m_ButtonSize)) //Clearly some wrong offset going on here. ImGui::GetWindowSize().y / 2)))
		{


		};
	};

	if (ImGui::BeginViewportSideBar("##MainToolsBar", nullptr, ImGuiDir_Up, height, windowFlags)) //Specifies that this will be pipped at the top of the window, below the main menu bar.
	{
		ImGui::SameLine(ImGui::GetWindowSize().x / 2); //All our icons are to be on the same line. 
		DisplayToolbarIcon(IconType::IconType_Toolbar_Play);

		ImGui::SameLine((ImGui::GetWindowSize().x / 2) - 50.0f);
		DisplayToolbarIcon(IconType::IconType_Toolbar_Pause);

		ImGui::SameLine((ImGui::GetWindowSize().x / 2) + 50.0f);
		DisplayToolbarIcon(IconType::IconType_Toolbar_Stop);

		ImGui::End();
	}
}