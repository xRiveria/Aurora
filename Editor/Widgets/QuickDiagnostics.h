#pragma once
#include "../Backend/Widget.h"
#include "../Renderer/Renderer.h"

struct _MEMORYSTATUSEX;

class QuickDiagnostics : public Widget
{
public:
    QuickDiagnostics(Editor* editorContext, Aurora::EngineContext* engineContext);
    
    void OnTickVisible() override;

private:
    void RetrieveProcessorInformation();
    float GetCurrentPhysicalMemoryUsage();

private:
    char m_CPU_BrandString[0x40];
    float m_PhysicalMemoryTotal;
    MEMORYSTATUSEX m_MemoryStatus;

    Aurora::Renderer* m_Renderer;
};