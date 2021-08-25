#include "Toolbar.h"
#include "../Backend/Source/imgui_internal.h"
#include "../Backend/Utilities/Extensions.h"
#include <functional>

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

	// By adding this toolbar, we are essentially implementing game/editor mode toggling. Hence, we will explictly start off with game mode disabled, even though the core engine builds with it enabled by default.
	m_EngineContext->GetEngine()->EngineFlag_Disable(Aurora::EngineFlag::EngineFlag_TickGame);
}

void Toolbar::OnTickAlways()
{
	Aurora::AURORA_PROFILE_FUNCTION();

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
	float height = ImGui::GetFrameHeight() + 18.0f;

	const auto DisplayToolbarIcon = [this](const IconType& iconType, const std::function<bool()>& GetVisibility, const std::function<void()>& Function)
	{
		//ImGui::SameLine(); //All our icons are to be on the same line. 
		ImGui::PushStyleColor(ImGuiCol_Button, GetVisibility() ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] : ImGui::GetStyle().Colors[ImGuiCol_Button]);
		if (EditorExtensions::ImageButton(iconType, m_ButtonSize)) //Clearly some wrong offset going on here. ImGui::GetWindowSize().y / 2)))
		{
			Function();
		};
		ImGui::PopStyleColor();
	};

	if (ImGui::BeginViewportSideBar("##MainToolsBar", nullptr, ImGuiDir_Up, height, windowFlags)) //Specifies that this will be pipped at the top of the window, below the main menu bar.
	{
		ImGui::SameLine(ImGui::GetWindowSize().x / 2); //All our icons are to be on the same line. 
		DisplayToolbarIcon(IconType::IconType_Toolbar_Play, [this]() { return m_EngineContext->GetEngine()->EngineFlag_IsSet(Aurora::EngineFlag::EngineFlag_TickGame); }, [this]() { m_EngineContext->GetEngine()->EngineFlag_Enable(Aurora::EngineFlag::EngineFlag_TickGame); });

		ImGui::SameLine((ImGui::GetWindowSize().x / 2) - 50.0f);
		DisplayToolbarIcon(IconType::IconType_Toolbar_Pause, [this]() { return !m_EngineContext->GetEngine()->EngineFlag_IsSet(Aurora::EngineFlag::EngineFlag_TickGame); }, [this]() { m_EngineContext->GetEngine()->EngineFlag_Disable(Aurora::EngineFlag::EngineFlag_TickGame); });

		ImGui::SameLine((ImGui::GetWindowSize().x / 2) + 50.0f);
		DisplayToolbarIcon(IconType::IconType_Toolbar_Stop, [this]() { return !m_EngineContext->GetEngine()->EngineFlag_IsSet(Aurora::EngineFlag::EngineFlag_TickGame); }, [this]() { m_EngineContext->GetEngine()->EngineFlag_Disable(Aurora::EngineFlag::EngineFlag_TickGame); });

		ImGui::End();
	}
}