#pragma once
#include "../Backend/Widget.h"

class MenuBar : public Widget
{
public:
    MenuBar(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickAlways() override;
    void OnEvent(Aurora::InputEvent& inputEvent) override;

    void ShowAboutWindow();

private:
    bool OnKeyPressed(Aurora::KeyPressedEvent& keyPressedEvent);

    void SaveScene();
    void LoadScene();
    void NewScene();

private:
    bool m_IsShowingAboutWindow = false;
};