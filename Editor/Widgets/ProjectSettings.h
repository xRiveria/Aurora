#pragma once
#include "../Backend/Widget.h"
#include "../Backend/Utilities/Extensions.h"

namespace Aurora
{
    class Settings;
}

using namespace EditorExtensions;
class ProjectSettings : public Widget
{
public:
    ProjectSettings(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickVisible() override;

private:
    Aurora::Settings* m_SettingsSubsystem;
};