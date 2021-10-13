#include "EditorLevelContext.h"
#include "Backend/Editor.h"
#include "Widgets/Hierarchy.h"
#include "Widgets/Properties.h"
#include "Widgets/Viewport.h"
#include "Widgets/ObjectsPanel.h"

namespace AuroraEditor
{
    EditorLevelContext::EditorLevelContext(Editor* pEditor, Aurora::EngineContext* pEngine)
                       : EditorContext(pEditor, pEngine, EditorContext_Type::EditorContext_Type_Level)
    {

    }

    bool EditorLevelContext::OnInitialize()
    {
        m_Widgets.emplace_back(m_Editor->GetWidget<Hierarchy>());
        m_Widgets.emplace_back(m_Editor->GetWidget<Properties>());
        m_Widgets.emplace_back(m_Editor->GetWidget<Viewport>());
        m_Widgets.emplace_back(m_Editor->GetWidget<ObjectsPanel>());

        return true;
    }

    void EditorLevelContext::OnTick(float deltaTime)
    {
        EditorContext::OnTick(deltaTime);
    }
}