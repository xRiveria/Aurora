#pragma once
#include "AuroraObject.h"
#include "../RHI_Implementation.h"

namespace Aurora
{
    enum DX11_ResourceViewFlag : uint32_t // #
    {
        Texture_Flag_SRV = 1 << 0,       // Shader Resource View
        Texture_Flag_UAV = 1 << 1,       // Unordered Access View
        Texture_Flag_RTV = 1 << 2,       // Render Target View
        Texture_Flag_DSV = 1 << 3,       // Depth Stencil View
    };

    class DX11_Texture : public AuroraObject
    {
    public:
        DX11_Texture() = default;
        ~DX11_Texture();

        bool Initialize(uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, uint32_t sampleLevels, uint32_t textureFlags, uint32_t mipSlice, DX11_Devices* devices);

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetSampleLevel() const { return m_SampleLevel; }
        DXGI_FORMAT GetFormat() const { return m_Format; }

        // Resource Views
        ComPtr<ID3D11Texture2D> GetTexture() const { return m_Texture; }
        ComPtr<ID3D11ShaderResourceView> GetShaderResourceView();
        ComPtr<ID3D11DepthStencilView> GetDepthStencilView();
        ComPtr<ID3D11UnorderedAccessView> GetUnorderedAccessView();
        ComPtr<ID3D11RenderTargetView> GetRenderTargetView();

        bool HasResourceView(DX11_ResourceViewFlag shaderViewType);

    private:
        void _DestroyTexture();

    private:
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_SampleLevel = 0;
        DXGI_FORMAT m_Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

        ComPtr<ID3D11Texture2D> m_Texture = nullptr;

        ComPtr<ID3D11RenderTargetView> m_RenderTargetView = nullptr;
        ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView = nullptr;
        ComPtr<ID3D11DepthStencilView> m_DepthStencilView = nullptr;
        ComPtr<ID3D11UnorderedAccessView> m_UnorderedAccessView = nullptr;
        uint32_t m_ResourceViewFlags = 0;

        DX11_Devices* m_Devices = nullptr;
    };
}