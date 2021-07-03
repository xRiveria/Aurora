#pragma once
#include "EngineContext.h"
#include "../Window/WindowContext.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace Aurora
{
    class Canvas  // The canvas specifies a DPI-aware drawing area.
    {
    public:
        // Create a canvas from physical measurements.
        void Initialize(uint32_t width, uint32_t height, float dpi = 96, EngineContext* engineContext = nullptr)
        {
            this->m_Width = width;
            this->m_Height = height;
            this->m_DPI = dpi;
            this->m_EngineContext = engineContext;
        }

        // Copy canvas from another canvas.
        void Initialize(const Canvas& otherCanvas)
        {
            *this = otherCanvas;
        }

        // Create the canvas straight from a system window.
        void Initialize(void* window, EngineContext* engineContext = nullptr)
        {
            this->m_Width = m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(window);
            this->m_Height = m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(window);
            this->m_DPI = m_EngineContext->GetSubsystem<WindowContext>()->GetWindowDPI(window);
            this->m_EngineContext = engineContext;
        }

        float GetDPI() const { return m_DPI; }                      // How many pixels are there per inch?
        float GetDPIScaling() const { return GetDPI() / 96.0f; }    // The scaling factor between logical and physical coordinates.
        
        // Returns the native resolution width/height in pixels. Use this for texture allocations, scissors and viewports.
        uint32_t GetPhysicalWidth() const { return m_Width; }
        uint32_t GetPhysicalHeight() const { return m_Height; }

        // Returns the width/height with DPI scaling applied (subpixel size). Use this for logic and positioning drawing elements.
        uint32_t GetLogicalWidth() const { return GetPhysicalWidth() / GetDPIScaling(); }
        uint32_t GetLogicalHeight() const { return GetPhysicalHeight() / GetDPIScaling(); }

        // Returns a projection matrix that maps logical space to physical device. Use this to render to a graphics viewport.
        XMMATRIX GetProjection() const
        {
            return XMMatrixOrthographicOffCenterLH(0, (float)GetLogicalWidth(), (float)GetLogicalHeight(), 0, -1, 1);
        }

    private:
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        float m_DPI = 96;           // By default, Windows has a setting of 96 API. DPI is the number of individual dots that can be placed in a line within the span of 1 inch (2.54cm).

        EngineContext* m_EngineContext = nullptr;
    };
}