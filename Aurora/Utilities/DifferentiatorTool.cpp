#include "Aurora.h"
#include "DifferentiatorTool.h"

namespace Aurora
{
    std::vector<DT_Entry> DifferentiatorTool::m_History;
    int DifferentiatorTool::m_CurrentHistoryIndex = -1; // Serves as an index into our m_History vector, in which our users will undo and redo upon.

    void DifferentiatorTool::PushAction(float oldValue, float newValue, std::function<void(std::any)> Setter)
    {
        DT_Entry newEntry;
        newEntry.m_OldValue = oldValue;
        newEntry.m_NewValue = newValue;
        newEntry.Setter = std::move(Setter);

        m_History.emplace_back(std::move(newEntry));
        m_CurrentHistoryIndex = m_History.size() - 1; // Use -1 here so our index can be used as a direct sample for positioning within our vector.
    }

    void DifferentiatorTool::Undo()
    {
        if (m_CurrentHistoryIndex >= 0)
        {
            const DT_Entry& retrievedEntry = m_History[m_CurrentHistoryIndex];
            retrievedEntry.Setter(retrievedEntry.m_OldValue);
            m_CurrentHistoryIndex--;
        }
        else
        {
            return;
        }
    }

    void DifferentiatorTool::Redo()
    {
        if ((m_CurrentHistoryIndex + 1) < m_History.size())
        {
            m_CurrentHistoryIndex++;
            const DT_Entry& retrievedEntry = m_History[m_CurrentHistoryIndex];
            retrievedEntry.Setter(retrievedEntry.m_NewValue);
        }
        else
        {
            return;
        }
    }

    void DifferentiatorTool::ClearHistory()
    {
        if (!m_History.empty())
        {
            std::cout << "Successfully cleared DT History.\n";
            m_History.clear();
            m_History.shrink_to_fit();
        }
    }
}