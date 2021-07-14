#pragma once
#include "../Backend/Widget.h"
#include "../Renderer/Renderer.h"

class QuickDiagnostics : public Widget
{
public:
    QuickDiagnostics(Editor* editorContext, Aurora::EngineContext* engineContext);
    
    void OnTickVisible() override;

private:
    void RetrieveProcessorInformation();

private:
    char m_CPU_BrandString[0x40];
    float m_PhysicalMemoryTotal;

    Aurora::Renderer* m_Renderer;
};