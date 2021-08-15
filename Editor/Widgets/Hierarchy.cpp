#include "Hierarchy.h"
#include "../Scene/World.h"
#include "../Scene/Components/Mesh.h"
#include "../Input/Input.h"
#include "../Input/InputUtilities.h"
#include "../Backend/Source/imgui_internal.h"
#include "../Backend/Utilities/Extensions.h"
#include "Properties.h"

namespace HierarchyGlobals
{
    static Aurora::World* g_WorldSubsystem = nullptr;
    static Aurora::Input* g_InputSubsystem = nullptr;

    // Entities in relation to mouse events.
    static Aurora::Entity* g_Entity_Clicked = nullptr;
    static Aurora::Entity* g_Entity_Hovered = nullptr;

    bool m_Expand_To_Selection = false;
    bool m_Expanded_To_Selection = false;
    ImRect m_Selected_Entity_Rect;
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

void Hierarchy::OnEvent(Aurora::InputEvent& inputEvent)
{
    Aurora::InputEventDispatcher dispatcher(inputEvent);
    dispatcher.Dispatch<Aurora::KeyPressedEvent>(AURORA_BIND_INPUT_EVENT(Hierarchy::OnKeyPressed));
}

bool Hierarchy::OnKeyPressed(Aurora::KeyPressedEvent& inputEvent)
{
    if (inputEvent.GetRepeatCount() > 0)
    {
        return false;
    }

    switch (inputEvent.GetKeyCode())
    {
        case AURORA_KEY_TAB:
            if (EditorExtensions::ContextHelper::GetInstance().m_SelectedEntity.lock())
            {
                if (const Aurora::Mesh* entityWithMesh = EditorExtensions::ContextHelper::GetInstance().m_SelectedEntity.lock()->GetComponentInChildren<Aurora::Mesh>())
                {
                    EditorExtensions::ContextHelper::GetInstance().SetSelectedEntity(entityWithMesh->GetEntity()->GetPointerShared());
                    Properties::m_InspectedEntity = entityWithMesh->GetEntity()->GetPointerShared();
                    return true;
                }
            }
            break;
    }

    return false;
}

void Hierarchy::ShowEntityTree()
{
    OnTreeBegin();

    /// Dropping on the scene node should unparent the entity.
    
    std::vector<std::shared_ptr<Aurora::Entity>> rootEntities = HierarchyGlobals::g_WorldSubsystem->EntityGetRoots();
    
    for (const std::shared_ptr<Aurora::Entity>& entity : rootEntities)
    {
        TreeAddEntity(entity.get());
    }
    
    // If we have been expanding to show an entity and no more expansions are taking place, we're done with it. Thus, we stop expanding and bring it into view.
    if (HierarchyGlobals::m_Expand_To_Selection && !HierarchyGlobals::m_Expanded_To_Selection)
    {
        // ImGui::ScrollToBringRectIntoView()
        HierarchyGlobals::m_Expanded_To_Selection = false;
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
    HandlePopups();
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
    if (isRightClicked)
    {
        if (HierarchyGlobals::g_Entity_Hovered)
        {
            SetSelectedEntity(HierarchyGlobals::g_Entity_Hovered->GetPointerShared());
        }

        ImGui::OpenPopup("##HierarchyContextMenu");
    }

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

    HierarchyGlobals::m_Expanded_To_Selection = false;
    bool isSelectedEntity = false;
    const bool isVisibleInHierarchy = entity->IsVisibleInHierarchy();
    bool hasVisibleChildren = false;

    // Dont't draw invisible entities.
    if (!isVisibleInHierarchy)
    {
        return;
    }

    // Determine children visibility.
    const std::vector<Aurora::Transform*>& childTransforms = entity->GetTransform()->GetChildren();
    for (Aurora::Transform* child : childTransforms)
    {
        if (child->GetEntity()->IsVisibleInHierarchy())
        {
            hasVisibleChildren = true;
            break;
        }
    }

    // Flags
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanFullWidth;

    // Flags - Is Expandable (Has Children)
    nodeFlags |= hasVisibleChildren ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf;

    // Flag - Is Selected?
    if (const auto selectedEntity = EditorExtensions::ContextHelper::GetInstance().m_SelectedEntity.lock())
    {
        nodeFlags |= selectedEntity->GetObjectID() == entity->GetObjectID() ? ImGuiTreeNodeFlags_Selected : nodeFlags;

        if (HierarchyGlobals::m_Expand_To_Selection)
        {
            // If the selected entity is a descendant of this entity, start expanding (this can happen if any entity is selected in the viewport).
            if (selectedEntity->GetTransform()->IsDescendantOf(entity->GetTransform()))
            {
                ImGui::SetNextItemOpen(true);
                HierarchyGlobals::m_Expanded_To_Selection = true;
            }
        }
    }

    const bool isNodeOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity->GetObjectID())), nodeFlags, entity->GetObjectName().c_str());

    // Keep a copy of the selected item's rect so that we can scroll to bring it to view.
    if ((nodeFlags & ImGuiTreeNodeFlags_Selected) && HierarchyGlobals::m_Expand_To_Selection)
    {
        /// 
        //HierarchyGlobals::m_Selected_Entity_Rect = m_Window->DC.LastItemRect;
    }

    // Manually detect some useful states.
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
    {
        HierarchyGlobals::g_Entity_Hovered = entity;
    }

    /// Entity Handle Drag Drop

    // Recursively show all child nodes.
    if (isNodeOpen)
    {
        if (hasVisibleChildren)
        {
            for (const auto& child : childTransforms)
            {
                if (!child->GetEntity()->IsVisibleInHierarchy())
                {
                    continue;
                }

                TreeAddEntity(child->GetEntity());
            }
        }

        // Pop if node is open.
        ImGui::TreePop();
    }
}

void Hierarchy::SetSelectedEntity(const std::shared_ptr<Aurora::Entity>& entity, bool fromEditor)
{
    HierarchyGlobals::m_Expand_To_Selection = true;

    // If the update came from this widget, let the engine know about it.
    if (fromEditor)
    {
        EditorExtensions::ContextHelper::GetInstance().SetSelectedEntity(entity);
    }

    Properties::Inspect(entity);
}

void Hierarchy::HandlePopups()
{
    PopupContextMenu();
}

void Hierarchy::PopupContextMenu()
{
    if (!ImGui::BeginPopup("##HierarchyContextMenu"))
    {
        return;
    }

    const auto selectedEntity = EditorExtensions::ContextHelper::GetInstance().m_SelectedEntity.lock();
    const bool isEntitySelected = selectedEntity != nullptr;

    if (isEntitySelected)
    {
        if (ImGui::MenuItem("Clone"))
        {
            selectedEntity->Clone();
        }
    }

    ImGui::EndPopup();
}
