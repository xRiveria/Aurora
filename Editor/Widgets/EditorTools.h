#pragma once
#include "../Backend/Widget.h"

// Should be rendered last (added last) to our widget vector as it may depend on widgets that come before it within its frame.

class EditorTools : public Widget
{
public:
    EditorTools(Editor* editorContext, Aurora::EngineContext* engineContext);
    
    void OnTickViewport();
    void OnEvent(Aurora::InputEvent& inputEvent) override;

    void TickGizmos();

private:
    bool OnKeyPressed(Aurora::KeyPressedEvent& inputEvent);

private:
    int m_GizmoType = -1;
};