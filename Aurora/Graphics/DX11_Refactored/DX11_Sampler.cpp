#include "Aurora.h"
#include "DX11_Sampler.h"

namespace Aurora
{
    DX11_Sampler::~DX11_Sampler()
    {
        if (m_Sampler != nullptr)
        {
            m_Sampler.Reset();
        }
    }

    bool DX11_Sampler::Initialize(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, D3D11_COMPARISON_FUNC comparisonFunction, float mipLODBias, float borderColor, DX11_Devices* devices)
    {
        AURORA_ASSERT(devices != nullptr);
        m_Devices = devices;

        D3D11_SAMPLER_DESC samplerDescription = {};
        samplerDescription.AddressU = addressMode;
        samplerDescription.AddressV = addressMode;
        samplerDescription.AddressW = addressMode;
        samplerDescription.Filter = filter;
        samplerDescription.ComparisonFunc = comparisonFunction;
        samplerDescription.MaxAnisotropy = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
        samplerDescription.BorderColor[0] = borderColor;
        samplerDescription.BorderColor[1] = borderColor;
        samplerDescription.BorderColor[2] = borderColor;
        samplerDescription.BorderColor[3] = borderColor;
        samplerDescription.MipLODBias = mipLODBias;
        samplerDescription.MinLOD = 0;
        samplerDescription.MaxLOD = FLT_MAX;

        const HRESULT result = m_Devices->m_Device->CreateSamplerState(&samplerDescription, m_Sampler.GetAddressOf());
        if (FAILED(result))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Sampler.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Failed to create Sampler");
        return true;
    }
}