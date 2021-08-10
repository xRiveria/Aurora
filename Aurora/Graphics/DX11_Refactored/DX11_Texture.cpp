#include "Aurora.h"
#include "DX11_Texture.h"
#include "DX11_Utilities.h"

namespace Aurora
{
    DX11_Texture::~DX11_Texture()
    {
        _DestroyTexture();
    }

    bool DX11_Texture::Initialize(uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, uint32_t sampleLevels, uint32_t textureFlags, uint32_t mipSlice, DX11_Devices* devices)
    {
        AURORA_ASSERT(devices != nullptr);

        m_Devices = devices;
        m_Width = textureWidth;
        m_Height = textureHeight;
        m_Format = format;
        m_SampleLevel = sampleLevels;
        m_ResourceViewFlags = textureFlags;

        D3D11_TEXTURE2D_DESC textureDescription = {};
        textureDescription.Width = static_cast<UINT>(textureWidth);
        textureDescription.Height = static_cast<UINT>(textureHeight);
        textureDescription.MipLevels = static_cast<UINT>(1);
        textureDescription.ArraySize = static_cast<UINT>(1);
        textureDescription.SampleDesc.Count = static_cast<UINT>(sampleLevels);
        textureDescription.SampleDesc.Quality = 0;

        if (format != DXGI_FORMAT_UNKNOWN)
        {
            textureDescription.Format = format;
            textureDescription.BindFlags = ParseBindFlags(textureFlags);
        }

        if (FAILED(m_Devices->m_Device->CreateTexture2D(&textureDescription, nullptr, m_Texture.GetAddressOf())))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Texture.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created 2D Texture.");

        if (textureFlags & DX11_ResourceViewFlag::Texture_Flag_SRV)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription = {};
            shaderResourceViewDescription.Format = textureDescription.Format;
            shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            shaderResourceViewDescription.Texture2D.MostDetailedMip = 0;
            shaderResourceViewDescription.Texture2D.MipLevels = 1;

            if (FAILED(m_Devices->m_Device->CreateShaderResourceView((ID3D11Resource*)m_Texture.Get(), &shaderResourceViewDescription, m_ShaderResourceView.GetAddressOf())))
            {
                AURORA_ERROR(LogLayer::Graphics, "Failed to create Shader Resource View");
                return false;
            }

            AURORA_INFO(LogLayer::Graphics, "Successfully created Shader Resource View.");
        }

        if (textureFlags & DX11_ResourceViewFlag::Texture_Flag_RTV)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDescription = {};
            renderTargetViewDescription.Format = textureDescription.Format;
            renderTargetViewDescription.ViewDimension = (sampleLevels > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

            if (FAILED(m_Devices->m_Device->CreateRenderTargetView((ID3D11Resource*)m_Texture.Get(), &renderTargetViewDescription, m_RenderTargetView.GetAddressOf())))
            {
                AURORA_ERROR(LogLayer::Graphics, "Failed to create Render Target View.");
                return false;
            }

            AURORA_INFO(LogLayer::Graphics, "Successfully created Render Target View.");
        }

        if (textureFlags & DX11_ResourceViewFlag::Texture_Flag_DSV)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription = {};
            depthStencilViewDescription.Format = textureDescription.Format;
            depthStencilViewDescription.ViewDimension = (sampleLevels > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

            if (FAILED(m_Devices->m_Device->CreateDepthStencilView((ID3D11Resource*)m_Texture.Get(), &depthStencilViewDescription, m_DepthStencilView.GetAddressOf())))
            {
                AURORA_ERROR(LogLayer::Graphics, "Failed to create Depth Stencil View");
                return false;
            }

            AURORA_INFO(LogLayer::Graphics, "Successfully created Depth Stencil View.");
        }

        if (textureFlags & DX11_ResourceViewFlag::Texture_Flag_UAV)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDescription = {};
            unorderedAccessViewDescription.Format = textureDescription.Format;

            if (textureDescription.ArraySize == 1)
            {
                unorderedAccessViewDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                unorderedAccessViewDescription.Texture2D.MipSlice = mipSlice;
            }
            else
            {
                unorderedAccessViewDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                unorderedAccessViewDescription.Texture2DArray.MipSlice = mipSlice;
                unorderedAccessViewDescription.Texture2DArray.FirstArraySlice = 0;
                unorderedAccessViewDescription.Texture2DArray.ArraySize = textureDescription.ArraySize;
            }

            if (FAILED(m_Devices->m_Device->CreateUnorderedAccessView((ID3D11Resource*)m_Texture.Get(), &unorderedAccessViewDescription, m_UnorderedAccessView.GetAddressOf())))
            {
                AURORA_ERROR(LogLayer::Graphics, "Failed to create Unordered Access View.");
                return false;
            }

            AURORA_INFO(LogLayer::Graphics, "Successfully created Unordered Access View.");
        }

        return true;
    }

    bool DX11_Texture::HasResourceView(DX11_ResourceViewFlag shaderViewType)
    {
        return shaderViewType & m_ResourceViewFlags;
    }

    ComPtr<ID3D11ShaderResourceView> DX11_Texture::GetShaderResourceView()
    {
        if (HasResourceView(DX11_ResourceViewFlag::Texture_Flag_SRV))
        {
            return m_ShaderResourceView;
        }

        AURORA_ERROR(LogLayer::Graphics, "Failed to retrieve Shader Resource View.");
        return nullptr;
    }

    ComPtr<ID3D11DepthStencilView> DX11_Texture::GetDepthStencilView()
    {
        if (HasResourceView(DX11_ResourceViewFlag::Texture_Flag_DSV))
        {
            return m_DepthStencilView;
        }

        AURORA_ERROR(LogLayer::Graphics, "Failed to retrieve Depth Stencil View.");
        return nullptr;
    }

    ComPtr<ID3D11UnorderedAccessView> DX11_Texture::GetUnorderedAccessView()
    {
        if (HasResourceView(DX11_ResourceViewFlag::Texture_Flag_UAV))
        {
            return m_UnorderedAccessView;
        }

        AURORA_ERROR(LogLayer::Graphics, "Failed to retrieve Unordered Access View.");
        return nullptr;
    }

    ComPtr<ID3D11RenderTargetView> DX11_Texture::GetRenderTargetView()
    {
        if (HasResourceView(DX11_ResourceViewFlag::Texture_Flag_RTV))
        {
            return m_RenderTargetView;
        }

        AURORA_ERROR(LogLayer::Graphics, "Failed to retrieve Render Target View.");
        return nullptr;
    }

    void DX11_Texture::_DestroyTexture()
    {
        if (m_Texture != nullptr)
        {
            m_Texture.Reset();
        }

        if (m_ShaderResourceView != nullptr)
        {
            m_ShaderResourceView.Reset();
        }

        if (m_UnorderedAccessView != nullptr)
        {
            m_UnorderedAccessView.Reset();
        }

        if (m_DepthStencilView != nullptr)
        {
            m_DepthStencilView.Reset();
        }

        if (m_RenderTargetView != nullptr)
        {
            m_RenderTargetView.Reset();
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully destroyed Texture and all associated views.");
    }
}