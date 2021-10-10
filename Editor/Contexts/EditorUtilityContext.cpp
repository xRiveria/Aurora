#include "EditorUtilityContext.h"
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
        m_Widgets.emplace_back(std::make_shared<ThreadTracker>(m_Editor, m_Engine));
        m_Widgets.emplace_back(std::make_shared<MathPlayground>(m_Editor, m_Engine));

        return true;
    }

    void EditorUtilityContext::OnTick(float deltaTime)
    {
        EditorContext::OnTick(deltaTime);
    }
}