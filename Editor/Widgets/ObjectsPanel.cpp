#include "ObjectsPanel.h"
#include "../Backend/Utilities/Extensions.h"
#include "../Resource/ResourceCache.h"

ObjectsPanel::ObjectsPanel(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Objects";
}

void ObjectsPanel::OnTickVisible()
{
    const auto ObjectEntry = [this](const std::string& objectName)
    {
        // Make work with our syntax.
        ImGui::Spacing();
        EditorExtensions::Image(IconType::IconType_ObjectPanel_Cube, m_IconSize);
        ImGui::SameLine();
        ImGui::Text(objectName.c_str());

        ObjectDrag(objectName);
    };

    ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("##Items", tabBarFlags))
    {
        if (ImGui::BeginTabItem("Default Shapes"))
        {
            ObjectEntry("Cube");
            ObjectEntry("Sphere");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Lighting"))
        {
            ObjectEntry("Point Light");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Camera"))
        {
            ObjectEntry("Perspective Camera");

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void ObjectsPanel::ObjectDrag(const std::string& objectType) const
{

}
