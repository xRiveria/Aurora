#include "Aurora.h"
#include "RenderTexture.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    RenderTexture::RenderTexture(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_Renderer = m_EngineContext->GetSubsystem<Renderer>();
    }

    RenderTexture::~RenderTexture()
    {
        if (m_ShaderResourceView)
        {
            m_ShaderResourceView->Release();
            m_ShaderResourceView = nullptr;
        }

        if (m_RenderTargetView)
        {
            m_RenderTargetView->Release();
            m_RenderTargetView = nullptr;
        }

        if (m_RenderTargetTexture)
        {
            m_RenderTargetTexture->Release();
            m_RenderTargetTexture = nullptr;
        }
    }

    bool RenderTexture::InitializeResources(int width, int height, int mipLevels)
    {
        m_TextureWidth = width;
        m_TextureHeight = height;

        D3D11_TEXTURE2D_DESC textureDescription;
        ZeroMemory(&textureDescription, sizeof(textureDescription));
        textureDescription.Width = width;
        textureDescription.Height = height;
        textureDescription.MipLevels = mipLevels;
        textureDescription.ArraySize = 1;
        textureDescription.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        textureDescription.SampleDesc.Count = 1;
        textureDescription.Usage = D3D11_USAGE_DEFAULT;
        textureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDescription.CPUAccessFlags = 0;
        textureDescription.MiscFlags = 0;

        // Create the texture.
        HRESULT result = m_Renderer->m_GraphicsDevice->m_Device->CreateTexture2D(&textureDescription, nullptr, &m_RenderTargetTexture);
        if (FAILED(result))
        {
            AURORA_ERROR("Failed to create Render Target Texture.");
            return false;
        }

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDescription;
        renderTargetViewDescription.Format = textureDescription.Format;
        renderTargetViewDescription.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDescription.Texture2D.MipSlice = 0;

        // Create the RTV.
        result = m_Renderer->m_GraphicsDevice->m_Device->CreateRenderTargetView(m_RenderTargetTexture, &renderTargetViewDescription, &m_RenderTargetView);
        if (FAILED(result))
        {
            AURORA_ERROR("Failed to create RTV.");
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription;
        shaderResourceViewDescription.Format = textureDescription.Format;
        shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDescription.Texture2D.MostDetailedMip = 0; // Index of the most detailed mipmap level.
        shaderResourceViewDescription.Texture2D.MipLevels = 1; // Maximum number of mipmap levels.

        // Create the SRV.
        result = m_Renderer->m_GraphicsDevice->m_Device->CreateShaderResourceView(m_RenderTargetTexture, &shaderResourceViewDescription, &m_ShaderResourceView);
        if (FAILED(result))
        {
            AURORA_ERROR("Failed to create SRV.");
            return false;
        }
    }

    void RenderTexture::SetRenderTarget(ID3D11DepthStencilView* depthStencilView) const
    {
        // Bind the render target view and depth stencil buffer to the output render pipeline.
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, &m_RenderTargetView, depthStencilView);
    }

    void RenderTexture::ClearRenderTarget(ID3D11DepthStencilView* depthStencilView, Vector3 clearColor) const
    {
        // Clear the back-buffer.
        float data[4] = { 0, 0, 0, 1 };
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->ClearRenderTargetView(m_RenderTargetView, data);

        // Clear the depth-buffer.
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);
    }
}