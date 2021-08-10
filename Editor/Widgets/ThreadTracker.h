#pragma once
#include "../Backend/Widget.h"

namespace Aurora
{
    class Threading;
}

class ThreadTracker : public Widget
{
public:
    ThreadTracker(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickVisible() override;

private:
    Aurora::Threading* m_ThreadingSubsystem;
};