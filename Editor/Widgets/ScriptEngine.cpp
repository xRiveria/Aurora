#include "ScriptEngine.h"
#include "../Scripting/ScriptInstance.h"
#include "../Scripting/Scripting.h"
#include <mono/jit/jit.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-gc.h>
#include "Settings.h"

using Aurora::ScriptMap;

ScriptEngine::ScriptEngine(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_ScriptingSubsystem = m_EngineContext->GetSubsystem<Aurora::Scripting>();
    m_WidgetName = "Script Engine";
}

void ScriptEngine::OnTickVisible()
{
    float gcHeapSize = (float)mono_gc_get_heap_size();
    float gcUsageSize = (float)mono_gc_get_used_size();
    ImGui::Text("GC Heap Info (Used/Avaliable): %.2fKB / %.2fKB", gcUsageSize / 1024.0f, gcHeapSize / 1024.0f);

    if (ImGui::Button("Hot Reload"))
    {
        m_ScriptingSubsystem->ReloadAssembly(m_EngineContext->GetSubsystem<Aurora::Settings>()->GetResourceDirectory(Aurora::ResourceDirectory::Scripts) + "\\Sandbox.dll");
    }

    /// Scene ID Segregation
    if (!Aurora::Scripting::s_ScriptInstanceMap.empty())
    {
        const uint32_t sceneID = 0;
        if (const auto& scriptInstanceMap = Aurora::Scripting::s_ScriptInstanceMap.find(sceneID); scriptInstanceMap != Aurora::Scripting::s_ScriptInstanceMap.end())
        {
            for (auto& [scriptEntityID, script] : scriptInstanceMap->second)
            {
                for (auto& [moduleName, fieldMap] : script.m_ScriptFieldMapping)
                {
                    bool opened = ImGui::TreeNode(moduleName.c_str());

                    if (opened)
                    {
                        for (auto& [fieldName, field] : fieldMap)
                        {
                            opened = ImGui::TreeNodeEx((void*)&field, ImGuiTreeNodeFlags_Leaf, fieldName.c_str());
                            if (opened)
                            {
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
        }
    }
}
