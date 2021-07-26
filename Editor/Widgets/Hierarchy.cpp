#include "Hierarchy.h"
#include "../Scene/World.h"
#include "../Input/Input.h"

namespace HierarchyGlobals
{
    static Aurora::World* g_WorldSubsystem = nullptr;
    static Aurora::Input* g_InputSubsystem = nullptr;

    // Entities in relation to mouse events.
    static Aurora::Entity* g_Entity_Clicked = nullptr;
    static Aurora::Entity* g_Entity_Hovered = nullptr;
}

Hierarchy::Hierarchy(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Hierarchy";
    m_WidgetFlags |= ImGuiWindowFlags_HorizontalScrollbar;

    HierarchyGlobals::g_WorldSubsystem = m_EngineContext->GetSubsystem<Aurora::World>();
    HierarchyGlobals::g_InputSubsystem = m_EngineContext->GetSubsystem<Aurora::Input>();
}

void Hierarchy::OnTickVisible()
{
    ShowEntityTree();

    // On left click, select the entity but only on release.
    if (ImGui::IsMouseReleased(0) && HierarchyGlobals::g_Entity_Clicked)
    {
        // Make sure that the mouse was released while on the same entity.
        if (HierarchyGlobals::g_Entity_Hovered && HierarchyGlobals::g_Entity_Hovered->GetObjectID() == HierarchyGlobals::g_Entity_Clicked->GetObjectID())
        {
            SetSelectedEntity(HierarchyGlobals::g_Entity_Clicked->GetPointerShared());
        }

        HierarchyGlobals::g_Entity_Clicked = nullptr;
    }
}

void Hierarchy::ShowEntityTree()
{
    OnTreeBegin();

    if (ImGui::TreeNodeEx("Root", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth))
    {
        /// Parent / Unparenting Stuff.

        std::vector<std::shared_ptr<Aurora::Entity>> rootEntities = HierarchyGlobals::g_WorldSubsystem->EntityGetAll();

        for (const std::shared_ptr<Aurora::Entity>& entity : rootEntities)
        {
            TreeAddEntity(entity.get());
        }

        /// Expansion Stuff.

        ImGui::TreePop();
    }

    OnTreeEnd();
}

void Hierarchy::OnTreeBegin()
{
    HierarchyGlobals::g_Entity_Hovered = nullptr;
}

void Hierarchy::OnTreeEnd()
{
    HandleClicking();
}

void Hierarchy::HandleClicking()
{
    const bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    const bool isLeftClicked = ImGui::IsMouseClicked(0);
    const bool isRightClicked = ImGui::IsMouseClicked(1);

    // Since we are handling clicking manually, we must ensure that we are inside the window.
    if (!isWindowHovered)
    {
        return;
    }

    // Left click on item - don't select it yet.
    if (isLeftClicked && HierarchyGlobals::g_Entity_Hovered)
    {
        HierarchyGlobals::g_Entity_Clicked = HierarchyGlobals::g_Entity_Hovered;
    }

    // Right click on item - select and show context menu.
    ///

    // Clicking on an empty space will clear our selection.
    if ((isLeftClicked || isRightClicked) && !HierarchyGlobals::g_Entity_Hovered)
    {
        SetSelectedEntity(m_Empty_Entity);
    }
}

void Hierarchy::TreeAddEntity(Aurora::Entity* entity)
{
    if (!entity)
    {
        return;
    }

    // bool isSelectedEntity = false;
    // const bool isVisibleInHierarchy = entity->
    bool hasVisibleChildren = false;
     
    // Flags
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanFullWidth;

    // Flags - Is Expandable (Has Children)
    nodeFlags |= hasVisibleChildren ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf;

    // Flag is selected.
}

void Hierarchy::SetSelectedEntity(const std::shared_ptr<Aurora::Entity>& entity, bool fromEditor)
{

}