#include "ObjectsPanel.h"
#include "../Resource/ResourceCache.h"

ObjectsPanel::ObjectsPanel(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Objects";
}

void ObjectsPanel::OnTickVisible()
{
    const auto ObjectEntry = [this](const Aurora::DefaultObjectType objectType, const std::string& objectName)
    {
        // Make work with our syntax.
        ImGui::Spacing();
        EditorExtensions::Image(IconType::IconType_ObjectPanel_Cube, m_IconSize);
        ImGui::SameLine();

        ImGui::Selectable(objectName.c_str());

        ObjectDrag(objectType, objectName);
    };

    ImGuiTabBarFlags tabBarFlags = ImGuiDockNodeFlags_None;

    if (ImGui::BeginTabBar("##Items", tabBarFlags))
    {
        if (ImGui::BeginTabItem("Default Shapes"))
        {
            ObjectEntry(Aurora::DefaultObjectType::DefaultObjectType_Cube, "Cube");
            ObjectEntry(Aurora::DefaultObjectType::DefaultObjectType_Sphere, "Sphere");
            ObjectEntry(Aurora::DefaultObjectType::DefaultObjectType_Cylinder, "Cylinder");
            ObjectEntry(Aurora::DefaultObjectType::DefaultObjectType_Plane, "Plane");
            ObjectEntry(Aurora::DefaultObjectType::DefaultObjectType_Torus, "Torus");
            ObjectEntry(Aurora::DefaultObjectType::DefaultObjectType_Cone, "Cone");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Lighting"))
        {
            ObjectEntry(Aurora::DefaultObjectType::DefaultObjectType_PointLight, "Point Light");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Camera"))
        {
            ObjectEntry(Aurora::DefaultObjectType::DefaultObjectType_Cube, "Perspective Camera");

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void ObjectsPanel::ObjectDrag(const Aurora::DefaultObjectType objectType, const std::string& objectName) const
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        const auto SetPayload = [this, objectType](const EditorExtensions::DragPayloadType payloadType)
        {
            m_DragDropPayload.m_PayloadType = payloadType;
            m_DragDropPayload.m_Data = objectType;
            EditorExtensions::CreateDragPayload(m_DragDropPayload);
        };

        SetPayload(EditorExtensions::DragPayloadType::DragPayloadType_Entity);

        // Preview.
        EditorExtensions::Image(IconType::IconType_ObjectPanel_Cube, 50.0f);
        ImGui::Text(objectName.c_str());

        ImGui::EndDragDropSource();
    }
}