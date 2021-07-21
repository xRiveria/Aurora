#pragma once
#include <string>
#include <DirectXMath.h>
#include "EngineContext.h"
#include "../Backend/Source/imgui.h"

class Editor;
class Aurora::EngineContext;

using namespace DirectX;

class Widget
{
public:
    Widget(Editor* editorContext, Aurora::EngineContext* engineContext);
    virtual ~Widget() = default;

    void Tick(); // Called across all Widgets.

    virtual void OnTickAlways()  {}    // Always called for all widgets, regardless of whether its a window or permanent aesthetic.
    virtual void OnTickVisible() {}    // Called only when the widget is visible. Certain widgets can be hidden or closed.

    // Properties
    void SetWidgetVisibility(bool isVisible) { m_IsWidgetVisible = isVisible; }

protected:
    bool m_IsWindowedWidget = true; // All widgets are initialized to be capable of becoming individual windows unless its a permanent widget (such as the menu bar).
    bool m_IsWidgetVisible = true;

    std::string m_WidgetName = "Widget";

    XMFLOAT2 m_WidgetSize = XMFLOAT2(-1.0, -1.0f);
    XMFLOAT2 m_WidgetPadding = XMFLOAT2(-1.0f, -1.0f);

    int m_WidgetFlags = ImGuiWindowFlags_NoCollapse; // Disable double clicking to collapse window.

    Aurora::EngineContext* m_EngineContext = nullptr;
    Editor* m_EditorContext = nullptr;
};