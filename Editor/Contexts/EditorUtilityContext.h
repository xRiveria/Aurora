#pragma once
#include "EditorContext.h"

namespace AuroraEditor
{
    class EditorUtilityContext : public EditorContext
    {
    public:
        EditorUtilityContext(Editor* pEditor, Aurora::EngineContext* pEngine);

        virtual bool OnInitialize() override;
        virtual void OnTick(float deltaTime) override;
    };
}