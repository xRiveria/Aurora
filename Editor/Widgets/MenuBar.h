#pragma once
#include "../Backend/Widget.h"

class MenuBar : public Widget
{
public:
    MenuBar(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickAlways() override;
};