#include "Aurora.h"
#include "DX11_Texture.h"
#include "DX11_Utilities.h"

namespace Aurora
{
    DX11_Texture::~DX11_Texture()
    {
        _DestroyTexture();
    }

    bool DX11_Texture::Initialize2DTexture(uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, uint32_t textureFlags, DX11_Devices* devices, uint32_t sampleLevels, uint32_t mipLevels, uint32_t mipSlice)
    {
        AURORA_ASSERT(devices != nullptr);

        _DestroyTexture(); // Destroy resources if they existed previously.

        m_Devices = devices;
        m_Width = textureWidth;
        m_Height = textureHeight;
        m_Format = format;
        m_SampleLevel = sampleLevels;
        m_MipLevels = mipLevels;
        m_ResourceViewFlags = textureFlags;

        D3D11_TEXTURE2D_DESC textureDescription = {};
        textureDescription.Width = static_cast<UINT>(textureWidth);
        textureDescription.Height = static_cast<UINT>(textureHeight);
        textureDescription.MipLevels = static_cast<UINT>(mipLevels);
        textureDescription.ArraySize = static_cast<UINT>(1);
        m_ArraySize = textureDescription.ArraySize;
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
            CreateShaderResourceView(D3D_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D);
        }

        if (textureFlags & DX11_ResourceViewFlag::Texture_Flag_RTV)
        {
            CreateRenderTargetView();
        }

        if (textureFlags & DX11_ResourceViewFlag::Texture_Flag_DSV)
        {
            CreateDepthStencilView();
        }

        if (m_ResourceViewFlags & DX11_ResourceViewFlag::Texture_Flag_UAV)
        {
            CreateUnorderedAccessView(mipSlice);
        }

        return true;
    }

    bool DX11_Texture::InitializeTextureCube(uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, DX11_Devices* devices, uint32_t mipLevels)
    {
        AURORA_ASSERT(devices != nullptr);
        
        m_Devices = devices;
        m_Width = textureWidth;
        m_Height = textureHeight;
        m_Format = format;
        m_MipLevels = mipLevels;

        D3D11_TEXTURE2D_DESC textureDescription = {};
        textureDescription.Width = textureWidth;
        textureDescription.Height = textureHeight;
        textureDescription.MipLevels = mipLevels;
        textureDescription.ArraySize = 6;
        m_ArraySize = textureDescription.ArraySize;
        textureDescription.Format = format;
        textureDescription.SampleDesc.Count = 1;
        textureDescription.Usage = D3D11_USAGE_DEFAULT;
        textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        textureDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        // This means that we would like to generate mips for this texture cube. In order for GenerateMips to work, we must specify BIND_RENDER_TARGET, BIND_SHADER_RESOURCE and RESOURCE_MISC_GENERATE_MIPS.
        if (mipLevels == 0)
        {
            textureDescription.BindFlags |= D3D11_BIND_RENDER_TARGET;
            textureDescription.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }

        if (FAILED(m_Devices->m_Device->CreateTexture2D(&textureDescription, nullptr, m_Texture.GetAddressOf())))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Texture Cube.");
            return false;
        }

        m_ResourceViewFlags = DX11_ResourceViewFlag::Texture_Flag_SRV | DX11_ResourceViewFlag::Texture_Flag_UAV;

        CreateShaderResourceView(D3D_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURECUBE);
        CreateUnorderedAccessView(0); 

        return true;
    }

    bool DX11_Texture::CreateUnorderedAccessView(uint32_t mipSlice)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDescription = {};
        unorderedAccessViewDescription.Format = m_Format;

        if (m_ArraySize == 1)
        {
            unorderedAccessViewDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            unorderedAccessViewDescription.Texture2D.MipSlice = mipSlice; // The slice index. A slice includes one mipmap level for every texture.
        }
        else
        {
            unorderedAccessViewDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
            unorderedAccessViewDescription.Texture2DArray.MipSlice = mipSlice;
            unorderedAccessViewDescription.Texture2DArray.FirstArraySlice = 0;
            unorderedAccessViewDescription.Texture2DArray.ArraySize = m_ArraySize;
        }

        if (m_UnorderedAccessView != nullptr)
        {
            m_UnorderedAccessView.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed UAV.");
        }

        if (FAILED(m_Devices->m_Device->CreateUnorderedAccessView((ID3D11Resource*)m_Texture.Get(), &unorderedAccessViewDescription, m_UnorderedAccessView.GetAddressOf())))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Unordered Access View.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Unordered Access View.");
        return true;
    }

    bool DX11_Texture::CreateShaderResourceView(D3D11_SRV_DIMENSION viewDimension)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription = {};

        shaderResourceViewDescription.Format = m_Format;
        if (m_Format == DXGI_FORMAT_R32G8X24_TYPELESS)
        {
            shaderResourceViewDescription.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;;
        }
        shaderResourceViewDescription.ViewDimension = viewDimension;

        if (viewDimension == D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D)
        {
            shaderResourceViewDescription.Texture2D.MostDetailedMip = 0;
            shaderResourceViewDescription.Texture2D.MipLevels = 1;
        }
        else if (viewDimension == D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURECUBE)
        {
            shaderResourceViewDescription.TextureCube.MostDetailedMip = 0;
            shaderResourceViewDescription.TextureCube.MipLevels = -1;
        }

        if (m_ShaderResourceView != nullptr)
        {
            m_ShaderResourceView.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed SRV.");
        }

        if (FAILED(m_Devices->m_Device->CreateShaderResourceView((ID3D11Resource*)m_Texture.Get(), &shaderResourceViewDescription, m_ShaderResourceView.GetAddressOf())))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Shader Resource View");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Shader Resource View.");
        return true;
    }

    bool DX11_Texture::CreateRenderTargetView()
    {
        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDescription = {};
        renderTargetViewDescription.Format = m_Format;
        renderTargetViewDescription.ViewDimension = (m_SampleLevel > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

        if (m_RenderTargetView != nullptr)
        {
            m_RenderTargetView.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed RTV.");
        }

        if (FAILED(m_Devices->m_Device->CreateRenderTargetView((ID3D11Resource*)m_Texture.Get(), &renderTargetViewDescription, m_RenderTargetView.GetAddressOf())))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Render Target View.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Render Target View.");
        return true;
    }

    bool DX11_Texture::CreateDepthStencilView()
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription = {};
        depthStencilViewDescription.Format = m_Format;
        if (m_Format == DXGI_FORMAT_R32G8X24_TYPELESS)
        {
            depthStencilViewDescription.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;;
        }
        depthStencilViewDescription.ViewDimension = (m_SampleLevel > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

        if (m_DepthStencilView != nullptr)
        {
            m_DepthStencilView.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed DSV.");
        }

        if (FAILED(m_Devices->m_Device->CreateDepthStencilView((ID3D11Resource*)m_Texture.Get(), &depthStencilViewDescription, m_DepthStencilView.GetAddressOf())))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Depth Stencil View");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Depth Stencil View.");
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
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed Texture.");
        }

        if (m_ShaderResourceView != nullptr)
        {
            m_ShaderResourceView.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed SRV.");
        }

        if (m_UnorderedAccessView != nullptr)
        {
            m_UnorderedAccessView.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed UAV.");
        }

        if (m_DepthStencilView != nullptr)
        {
            m_DepthStencilView.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed DSV.");
        }

        if (m_RenderTargetView != nullptr)
        {
            m_RenderTargetView.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully destroyed RTV.");
        }
    }
}