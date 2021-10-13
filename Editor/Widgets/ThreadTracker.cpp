#include "ThreadTracker.h"
#include "../Threading/Threading.h"

ThreadTracker::ThreadTracker(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_ThreadingSubsystem = m_EngineContext->GetSubsystem<Aurora::Threading>();
    m_WidgetName = "Threader";
}


void ThreadTracker::OnTickVisible()
{
    Aurora::AURORA_PROFILE_FUNCTION();

    ImGui::Text("Total Thread Count: %u", m_ThreadingSubsystem->GetThreadCount());
    ImGui::Text("Thread Count Supported: %u", m_ThreadingSubsystem->GetThreadCountSupported());
    ImGui::Text("Thread Count Avaliable: %u", m_ThreadingSubsystem->GetThreadCountAvaliable());
    ImGui::Text("Currently Queued Tasks: %u", m_ThreadingSubsystem->GetQueuedTasksCount());

    ImGui::Spacing();
    bool mainThreadTasking = m_ThreadingSubsystem->IsMainThreadUtilitizedForTasks();
    if (ImGui::Checkbox("Use Main Thread", &mainThreadTasking))
    {
        m_ThreadingSubsystem->UseMainThreadForTasks(mainThreadTasking);
    }
    ImGui::Spacing();

    if (ImGui::Button("Singular Unit Test"))
    {
        m_ThreadingSubsystem->SingularTaskUnitTest();
    }
    
    ImGui::SameLine();

    if (ImGui::Button("Loop Unit Test"))
    {
        m_ThreadingSubsystem->LoopingTaskUnitTest();
    }
}
