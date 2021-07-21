#pragma once
#include "../Backend/Widget.h"

// Should be rendered last (added last) to our widget vector as it may depend on widgets that come before it within its frame.

class EditorTools : public Widget
{
public:
    EditorTools(Editor* editorContext, Aurora::EngineContext* engineContext);
    
    void OnTickAlways() override;
    void TickGizmos();

private:
    int m_GizmoType = -1;
};