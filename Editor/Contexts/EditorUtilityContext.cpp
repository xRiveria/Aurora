#include "EditorUtilityContext.h"
#include "Backend/Editor.h"
#include "Widgets/ThreadTracker.h"
#include "Widgets/MathPlayground.h"
#include "Widgets/ScriptEngine.h"

namespace AuroraEditor
{
    EditorUtilityContext::EditorUtilityContext(Editor* pEditor, Aurora::EngineContext* pEngine) : EditorContext(pEditor, pEngine, EditorContext_Type::EditorContext_Type_Utility)
    {

    }

    bool EditorUtilityContext::OnInitialize()
    {
        m_Widgets.emplace_back(m_Editor->GetWidget<ThreadTracker>());
        m_Widgets.emplace_back(m_Editor->GetWidget<MathPlayground>());
        m_Widgets.emplace_back(m_Editor->GetWidget<ScriptEngine>());

        return true;
    }

    void EditorUtilityContext::OnTick(float deltaTime)
    {
        EditorContext::OnTick(deltaTime);
    }
}