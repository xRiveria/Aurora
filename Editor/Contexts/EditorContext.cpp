#include "EditorContext.h"
#include "Backend/Widget.h"

namespace AuroraEditor
{
    EditorContext::EditorContext(Editor* pEditor, Aurora::EngineContext* pEngine, EditorContext_Type contextType)
                  : m_Editor(pEditor), m_Engine(pEngine), m_Type(contextType)
    {

    }

    void EditorContext::OnTick(float deltaTime)
    {
        for (Widget* widget : m_Widgets)
        {
            widget->Tick();
        }
    }
}