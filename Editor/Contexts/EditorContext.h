#pragma once
#include <vector>
#include <memory>

namespace Aurora
{
    class EngineContext;
}

class Widget;
class Editor;

namespace AuroraEditor
{
    enum class EditorContext_Type
    {
        EditorContext_Type_Level,
        EditorContext_Type_VisualScripting,
        EditorContext_Type_Dialog,
        EditorContext_Type_VFX,
        EditorContext_Type_Utility,
        EditorContext_Type_Count
    };

    class EditorContext
    {
    public:
        EditorContext(Editor* pEditor, Aurora::EngineContext* pEngine, EditorContext_Type contextType);

        virtual bool OnInitialize() = 0;
        virtual void OnTick(float deltaTime);

    protected:
        std::vector<std::shared_ptr<Widget>> m_Widgets;
        EditorContext_Type m_Type = EditorContext_Type::EditorContext_Type_Count;

        // Contexts
        Editor* m_Editor = nullptr;
        Aurora::EngineContext* m_Engine = nullptr;
    };
}
