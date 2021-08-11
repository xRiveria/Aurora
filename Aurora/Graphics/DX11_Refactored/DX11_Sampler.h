#pragma once
#include "../Resource/AuroraObject.h"
#include "../RHI_Implementation.h"

namespace Aurora
{
    class DX11_Sampler : public AuroraObject
    {
    public:
        DX11_Sampler() = default;
        ~DX11_Sampler();

        bool Initialize(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, D3D11_COMPARISON_FUNC comparisonFunction, float mipLODBias, float borderColor, DX11_Devices* devices);

        ComPtr<ID3D11SamplerState> GetSampler() const { return m_Sampler; }

    private:
        ComPtr<ID3D11SamplerState> m_Sampler = nullptr;
        DX11_Devices* m_Devices = nullptr;
    };
}