#include "EditorLevelContext.h"
#include "Widgets/Hierarchy.h"
#include "Widgets/Properties.h"

namespace AuroraEditor
{
    EditorLevelContext::EditorLevelContext(Editor* pEditor, Aurora::EngineContext* pEngine)
                       : EditorContext(pEditor, pEngine, EditorContext_Type::EditorContext_Type_Level)
    {

    }

    bool EditorLevelContext::OnInitialize()
    {
        m_Widgets.emplace_back(std::make_shared<Hierarchy>(m_Editor, m_Engine));
        m_Widgets.emplace_back(std::make_shared<Properties>(m_Editor, m_Engine));

        return true;
    }

    void EditorLevelContext::OnTick(float deltaTime)
    {
        EditorContext::OnTick(deltaTime);
    }
}