#pragma once
#include "../Backend/Widget.h"
#include "../Math/Vector2.h"
#include "../Widgets/EditorTools.h"

namespace Aurora
{
    class Renderer;
    class Settings;
    class World;
    class Input;
}

class Viewport : public Widget
{
public:
    Viewport(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickVisible() override;

private:
    float m_Width = 0.0f;
    float m_Height = 0.0f;
    
    Aurora::Renderer* m_RendererSubsystem = nullptr;
    Aurora::World* m_WorldSubsystem = nullptr;
    Aurora::Input* m_InputSubsystem = nullptr;

    std::shared_ptr<EditorTools> m_EditorTools;
};