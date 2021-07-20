#pragma once
#include "../Backend/Widget.h"

class ObjectsPanel : public Widget
{
public:
    ObjectsPanel(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickVisible() override;

    // Object Panel Specific
    void ObjectDrag(const std::string& objectType) const;

private:
    float m_IconSize = 20.0f;

    // Internal
    /// Drag.
};