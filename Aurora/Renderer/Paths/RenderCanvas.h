#pragma once
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;
namespace Aurora
{
    class Settings;

    // The canvas specifies a DPI-aware drawing area.
    class RenderCanvas
    {
        // Create a canvas from physical measurements.
        void Initialize(uint32_t canvasWidth, uint32_t canvasHeight, float canvasDPI = 96)
        {
            this->m_CanvasWidth = canvasWidth;
            this->m_CanvasHeight = canvasHeight;
            this->m_DPI = canvasDPI;
        }

        // Create a canvas from system settings.
        void Initialize(Settings* settings)
        {

        }

        // Copy canvas from another canvas.
        void Initialize(const RenderCanvas& otherCanvas)
        {
            *this = otherCanvas;
        }

        float GetDPI() const { return m_DPI; }                       // How many pixels there are per inch?
        float GetDPIScaling() const { return GetDPI() / 96.0f; }     // Get the scaling factor between logical and physical coordinates. Returns 1 if there is no scaling.
        
        // Returns native resolution width/height in pixels - Use this for texture allocations, viewports, scissors etc.
        uint32_t GetPhysicalWidth() const { return m_CanvasWidth; }  
        uint32_t GetPhysicalHeight() const { return m_CanvasHeight; }

        // Returns the width and height with DPI scaling applied (subpixel size). Use this for logic and positioning drawable elements.
        float GetLogicalWidth() const { return GetPhysicalWidth() / GetDPIScaling(); }
        float GetLogicalHeight() const { return GetPhysicalHeight() / GetDPIScaling(); }

        // Returns projection matrix that maps logical to physical space. Use this to render to a graphics viewport.
        XMMATRIX GetProjectionMatrix() const
        {
            return XMMatrixOrthographicOffCenterLH(0, (float)GetLogicalWidth(), (float)GetLogicalHeight(), 0.0f, -1.0f, 1.0f);
        }

    private:
        uint32_t m_CanvasWidth = 0;
        uint32_t m_CanvasHeight = 0;
        float m_DPI = 96.0f;                // Windows OS have a default of 96 DPI.
    };
}