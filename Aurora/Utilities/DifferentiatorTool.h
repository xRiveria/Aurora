#pragma once
#include <any>
#include <queue>

namespace Aurora
{
    // Ctrl Z (Undo) and Ctrl Y (Redo).
    struct DT_Entry
    {
        float m_OldValue;
        float m_NewValue;
        std::function<void(std::any)> Setter;
    };

    class DifferentiatorTool
    {
    public:
        static void PushAction(float oldValue, float newValue, std::function<void(std::any)> Setter);
        static void Undo();
        static void Redo();
        static void ClearHistory(); 

    private:
        static int m_CurrentHistoryIndex;
        static std::vector<DT_Entry> m_History;
    };
};