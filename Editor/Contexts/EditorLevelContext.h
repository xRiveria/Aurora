#pragma once
#include "EditorContext.h"

namespace AuroraEditor
{
    class EditorLevelContext : public EditorContext
    {
    public:
        EditorLevelContext(Editor* pEditor, Aurora::EngineContext* pEngine);

        virtual bool OnInitialize() override;
        virtual void OnTick(float deltaTime) override;
    };
}