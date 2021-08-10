#pragma once
#include "AuroraObject.h"
#include "../RHI_Implementation.h"

namespace Aurora
{
    class DX11_RasterizerState : public AuroraObject
    {
    public:
        DX11_RasterizerState() = default;
        ~DX11_RasterizerState();

        bool Initialize(D3D11_RASTERIZER_DESC& rasterizerStateDescription, DX11_Devices* devices);

        ComPtr<ID3D11RasterizerState> GetRasterizerState() const { return m_RasterizerState; }

    private:
        ComPtr<ID3D11RasterizerState> m_RasterizerState = nullptr;
        DX11_Devices* m_Devices = nullptr;
    };
}