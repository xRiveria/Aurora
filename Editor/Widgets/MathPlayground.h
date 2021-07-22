#pragma once
#include "../Backend/Widget.h"

class MathPlayground : public Widget
{
public:
    MathPlayground(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickAlways() override;
};