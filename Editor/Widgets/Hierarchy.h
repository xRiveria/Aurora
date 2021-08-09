#pragma once
#include "../Backend/Widget.h"

namespace Aurora
{
    class Entity;
}

class Hierarchy : public Widget
{
public:
    Hierarchy(Editor* editorContext, Aurora::EngineContext* engineContext);
    void OnTickVisible() override;
    void OnEvent(Aurora::InputEvent& inputEvent) override;

private:
    // Tree
    void ShowEntityTree();
    void OnTreeBegin();
    void OnTreeEnd();
    void HandleClicking();
    void TreeAddEntity(Aurora::Entity* entity);
    void SetSelectedEntity(const std::shared_ptr<Aurora::Entity>& entity, bool fromEditor = true);

    void HandlePopups();
    void PopupContextMenu();
    bool OnKeyPressed(Aurora::KeyPressedEvent& inputEvent);

private:
    std::shared_ptr<Aurora::Entity> m_Empty_Entity;
};