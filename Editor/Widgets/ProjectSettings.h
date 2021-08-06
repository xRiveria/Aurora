#pragma once
#include "../Backend/Widget.h"

namespace Aurora
{
    class Settings;
}

class ProjectSettings : public Widget
{
public:
    ProjectSettings(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickVisible() override;

private:
    Aurora::Settings* m_SettingsSubsystem;
};