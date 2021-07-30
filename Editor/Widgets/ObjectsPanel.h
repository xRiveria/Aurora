#pragma once
#include "../Backend/Widget.h"
#include "../Backend/Utilities/Extensions.h"

class ObjectsPanel : public Widget
{
public:
    ObjectsPanel(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickVisible() override;

    // Object Panel Specific
    void ObjectDrag(const Aurora::DefaultObjectType objectType, const std::string& objectName) const;

private:
    float m_IconSize = 20.0f;

    // Internal
    mutable EditorExtensions::DragDropPayload m_DragDropPayload; // Currently operating payload.
};