#pragma once
#include <string>
#include "EngineContext.h"
#include "../Math/Vector2.h"
#include "../Backend/Source/imgui.h"
#include "../Input/Input.h"
#include "../Input/InputEvents/InputEvent.h"

class Editor;
class Aurora::InputEvent;
class Aurora::EngineContext;

class Widget
{
public:
    Widget(Editor* editorContext, Aurora::EngineContext* engineContext);
    virtual ~Widget() = default;

    void Tick(); // Called across all Widgets.

    virtual void OnTickAlways()  {}    // Always called for all widgets, regardless of whether its a window or permanent aesthetic.
    virtual void OnTickVisible() {}    // Called only when the widget is visible. Certain widgets can be hidden or closed.
    virtual void OnEvent(Aurora::InputEvent& inputEvent) {}          // Called when the widget itself listens to specific events, forwarded from the core engine.

    // Properties
    void SetWidgetVisibility(bool isVisible) { m_IsWidgetVisible = isVisible; }

protected:
    bool m_IsWindowedWidget = true; // All widgets are initialized to be capable of becoming individual windows unless its a permanent widget (such as the menu bar).
    bool m_IsWidgetVisible = true;

    std::string m_WidgetName = "Widget";

    Aurora::Math::Vector2 m_WidgetSize = Aurora::Math::Vector2(-1.0, -1.0f);
    float m_WidgetPadding = 6;

    int m_WidgetFlags = ImGuiWindowFlags_NoCollapse; // Disable double clicking to collapse window.

    Aurora::EngineContext* m_EngineContext = nullptr;
    Editor* m_EditorContext = nullptr;
};