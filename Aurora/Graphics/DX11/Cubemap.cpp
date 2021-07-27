#include "Aurora.h"
#include "Cubemap.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    Cubemap::Cubemap(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_Renderer = m_EngineContext->GetSubsystem<Renderer>();
    }

    Cubemap::~Cubemap()
    {
        if (m_Texture)
        {
            m_Texture->Release();
            m_Texture = nullptr;
        }

        if (m_ShaderResourceView)
        {
            m_ShaderResourceView->Release();
            m_ShaderResourceView = nullptr;
        }
    }

    bool Cubemap::InitializeResources(std::vector<std::shared_ptr<RenderTexture>> faces, int width, int height, int mipLevels)
    {
        m_MipLevels = mipLevels;

        D3D11_TEXTURE2D_DESC textureDescription;
        textureDescription.Width = width;
        textureDescription.Height = height;
        textureDescription.MipLevels = mipLevels;
        textureDescription.ArraySize = 6;
        textureDescription.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        textureDescription.SampleDesc.Count = 1;
        textureDescription.SampleDesc.Quality = 0;
        textureDescription.Usage = D3D11_USAGE_DEFAULT;
        textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDescription.CPUAccessFlags = 0;
        textureDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        HRESULT result = m_Renderer->m_GraphicsDevice->m_Device->CreateTexture2D(&textureDescription, nullptr, &m_Texture);
        if (FAILED(result))
        {
            AURORA_ERROR("Failed to create Texture Cube.");
            return false;
        }

        if (!faces.empty())
        {
            Copy(faces, width, height, 0);
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription;
        shaderResourceViewDescription.Format = textureDescription.Format;
        shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        shaderResourceViewDescription.TextureCube.MipLevels = textureDescription.MipLevels;
        shaderResourceViewDescription.TextureCube.MostDetailedMip = 0;

        result = m_Renderer->m_GraphicsDevice->m_Device->CreateShaderResourceView(m_Texture, &shaderResourceViewDescription, &m_ShaderResourceView);

        return !FAILED(result);
    }

    void Cubemap::Copy(std::vector<std::shared_ptr<RenderTexture>> faces, int width, int height, int mipSlice) const
    {
        D3D11_BOX sourceRegion;
        for (int i = 0; i < 6; i++)
        {
            RenderTexture* texture = faces[i].get();

            sourceRegion.left = 0;
            sourceRegion.right = width;
            sourceRegion.top = 0;
            sourceRegion.bottom = height;
            sourceRegion.front = 0;
            sourceRegion.back = 1;

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CopySubresourceRegion(m_Texture, D3D11CalcSubresource(mipSlice, i, m_MipLevels), 0, 0, 0, texture->GetTexture(), 0, &sourceRegion);
        }
    }
}