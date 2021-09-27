#pragma once
#include <any>
#include <queue>

namespace Aurora
{
    // Ctrl Z (Undo) and Ctrl Y (Redo).
    struct DT_Entry
    {
        std::any m_OldValue;
        std::any m_NewValue;
        std::function<void(const std::any& valueIn)> Setter;
    };

    class DifferentiatorTool
    {
    public:
        static void PushAction(const std::any& oldValue, const std::any& newValue, std::function<void(const std::any& valueIn)> Setter);
        static void Undo();
        static void Redo();
        static void ClearHistory(); 

    private:
        static int m_CurrentHistoryIndex;
        static std::vector<DT_Entry> m_History;
    };
};