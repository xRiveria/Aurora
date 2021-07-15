#include "QuickDiagnostics.h"
#include "FileSystem.h"
#include <sysinfoapi.h>

namespace FPS
{
    static double g_PreviousFrameTime = 0;
    static int g_FrameCount = 0;
    static double g_FramesPerSecond = 0;
}

QuickDiagnostics::QuickDiagnostics(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Diagnostics";
    m_Renderer = engineContext->GetSubsystem<Aurora::Renderer>();

	// CPU & RAM Retrieval
	m_MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&m_MemoryStatus);
	m_PhysicalMemoryTotal = (float)m_MemoryStatus.ullTotalPhys / 1048576;

	RetrieveProcessorInformation();
}

void QuickDiagnostics::OnTickVisible()
{
	ImGui::Text("CPU: %s", m_CPU_BrandString);
    ImGui::Text("GPU: %s", m_Renderer->m_GraphicsDevice->m_AdapterName.c_str());  // Vendor Name
	ImGui::Text("Physical Memory: %.0f MB / %.0f MB", GetCurrentPhysicalMemoryUsage(), m_PhysicalMemoryTotal);
    ImGui::Text("Graphics Memory: %.0f MB", m_Renderer->m_GraphicsDevice->m_GraphicsMemory);

    ImGui::Separator();

    ImGui::Text("Frame Rate (ImGui): %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void QuickDiagnostics::RetrieveProcessorInformation()
{
	int CPUInfo[4] = { -1 };
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];

	memset(m_CPU_BrandString, 0, sizeof(m_CPU_BrandString));

	// Get the information associated with each extended ID.
	for (int i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);

		// Interpret CPU brand string.
		if (i == 0x80000002)
			memcpy(m_CPU_BrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(m_CPU_BrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(m_CPU_BrandString + 32, CPUInfo, sizeof(CPUInfo));
	}
}

float QuickDiagnostics::GetCurrentPhysicalMemoryUsage()
{
	GlobalMemoryStatusEx(&m_MemoryStatus);
	return (m_MemoryStatus.ullTotalPhys - m_MemoryStatus.ullAvailPhys) / 1048576;
}
