#include "Aurora.h"
#include "DX11_RasterizerState.h"

namespace Aurora
{
    DX11_RasterizerState::~DX11_RasterizerState()
    {
        if (m_RasterizerState != nullptr)
        {
            m_RasterizerState.Reset();
        }
    }

    bool DX11_RasterizerState::Initialize(D3D11_RASTERIZER_DESC& rasterizerStateDescription, DX11_Devices* devices)
    {
        AURORA_ASSERT(devices != nullptr);
        m_Devices = devices;

        const HRESULT result = m_Devices->m_Device->CreateRasterizerState(&rasterizerStateDescription, m_RasterizerState.GetAddressOf());

        if (FAILED(result))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Rasterizer State.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Rasterizer State.");
        return true;
    }
}