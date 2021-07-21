#include "Widget.h"

Widget::Widget(Editor* editorContext, Aurora::EngineContext* engineContext) : m_EditorContext(editorContext), m_EngineContext(engineContext)
{

}

void Widget::Tick()
{
    // Constant Widgets - Has its own ImGui::Begin and ImGui::End if it is display related. For example, certain parts of EditorTools may not need to involve ImGui windows. Self contained and is always rendering.
    OnTickAlways();

    if (!m_IsWindowedWidget)
    {
        return;
    }


    if (!m_IsWidgetVisible)
    {
        return;
    }

    // Individal Widgets - Can be closed or open depending on the user. If closed, they are not visible and are hence returned above. Else, their ImGui::Begin and ImGui::Ends are nested here.
    ImGui::Begin(m_WidgetName.c_str(), &m_IsWidgetVisible, m_WidgetFlags);
    OnTickVisible();
    ImGui::End();
}