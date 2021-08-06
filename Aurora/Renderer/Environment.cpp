#include "Aurora.h"
#include "Environment.h"
#include "../Resource/ResourceCache.h"
#include "../Renderer/Renderer.h"
#include "../Scene/World.h"

namespace Aurora
{
    Environment::Environment(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_Renderer = m_EngineContext->GetSubsystem<Renderer>();

        CreateRenderResources();
    }

    void Environment::CreateRenderResources()
    {
        D3D11_RASTERIZER_DESC skyRasterizerState;
        skyRasterizerState.FillMode = D3D11_FILL_SOLID;
        skyRasterizerState.CullMode = D3D11_CULL_FRONT; // As we are inside a skybox, there is no need to see front facing triangles. Instead, we want to be able to see backfacing ones as our environmnet.
        skyRasterizerState.DepthClipEnable = true;
        m_Renderer->m_GraphicsDevice->m_Device->CreateRasterizerState(&skyRasterizerState, &m_SkyRasterizerState);

        D3D11_DEPTH_STENCIL_DESC depthStencilDescription = {};
        depthStencilDescription.DepthEnable = true;
        depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        m_Renderer->m_GraphicsDevice->m_Device->CreateDepthStencilState(&depthStencilDescription, &m_SkyDepthState);

        // Create Cube Entity
        m_SkyboxEntity = m_EngineContext->GetSubsystem<World>()->CreateDefaultObject(DefaultObjectType::DefaultObjectType_Sphere);
        m_SkyboxEntity->m_EntityType = EntityType::Skybox;
        m_SkyboxEntity->SetName("Skybox");

        for (int i = 0; i < m_SkyboxEntity->GetComponent<Transform>()->m_Children.size(); i++)
        {
            m_SkyboxEntity->GetComponent<Transform>()->m_Children[i]->GetEntity()->m_EntityType = EntityType::Skybox;
        }

        XMFLOAT3 position = XMFLOAT3(0, 0, 0);
        XMFLOAT4X4 cameraViewMatrix;
        XMFLOAT4X4 cameraProjectionMatrix;

        XMVECTOR cameraRotations[] = { XMVectorSet(0.0f, 90.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 270.0f, 0.0f, 0.0f), XMVectorSet(-90.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(90.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 180.0f, 0.0f, 0.0f) }; // front

        // ========================================================================================
        // Diffuse IBL
        D3D11_TEXTURE2D_DESC skyDescription;
        skyDescription.Width = 64;
        skyDescription.Height = 64;
        skyDescription.MipLevels = 1;
        skyDescription.ArraySize = 6;
        skyDescription.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        skyDescription.Usage = D3D11_USAGE_DEFAULT;
        skyDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        skyDescription.CPUAccessFlags = 0;
        skyDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
        skyDescription.SampleDesc.Count = 1;
        skyDescription.SampleDesc.Quality = 0;

        m_Renderer->m_GraphicsDevice->m_Device->CreateTexture2D(&skyDescription, 0, &m_SkyIBLTexture);

        ID3D11RenderTargetView* skyRTV[6];

        D3D11_SHADER_RESOURCE_VIEW_DESC skySRVDescription;
        ZeroMemory(&skySRVDescription, sizeof(skySRVDescription));
        skySRVDescription.Format = skyDescription.Format;
        skySRVDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        skySRVDescription.TextureCube.MostDetailedMip = 0;
        skySRVDescription.TextureCube.MipLevels = 1;

        if (m_SkyIBLTexture != nullptr)
        {
            m_Renderer->m_GraphicsDevice->m_Device->CreateShaderResourceView(m_SkyIBLTexture, &skySRVDescription, &m_SkySRVTexture);
        }

        D3D11_RENDER_TARGET_VIEW_DESC skyRTVDescription;
        ZeroMemory(&skyRTVDescription, sizeof(skyRTVDescription));
        skyRTVDescription.Format = skyDescription.Format;
        skyRTVDescription.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        skyRTVDescription.Texture2DArray.ArraySize = 1;
        skyRTVDescription.Texture2DArray.MipSlice = 0;

        D3D11_VIEWPORT skyViewport;
        skyViewport.Width = 64;
        skyViewport.Height = 64;
        skyViewport.MinDepth = 0.0f;
        skyViewport.MaxDepth = 1.0f;
        skyViewport.TopLeftX = 0.0f;
        skyViewport.TopLeftY = 0.0f;

        const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };

        for (int i = 0; i < 6; i++)
        {
            skyRTVDescription.Texture2DArray.FirstArraySlice = i;
            if (m_SkyIBLTexture != nullptr)
            {
                m_Renderer->m_GraphicsDevice->m_Device->CreateRenderTargetView(m_SkyIBLTexture, &skyRTVDescription, &skyRTV[i]);
            }

            XMFLOAT3 rotation;
            XMStoreFloat3(&rotation, cameraRotations[i]);
            m_Renderer->m_Camera->m_Transform->m_RotationInRadians = rotation;

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, &skyRTV[i], 0);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->RSSetViewports(1, &skyViewport);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->ClearRenderTargetView(skyRTV[i], color);


        }







        
        std::vector<ComPtr<ID3D11Resource>> textureResources;
        for (int i = 0; i < m_CubemapFaces.size(); i++)
        {
            Aurora::DX11_Utility::DX11_TexturePackage* textureSide = Aurora::DX11_Utility::ToInternal(&m_CubemapFaces[i]->m_Texture);
            textureResources.push_back(textureSide->m_Resource.Get());
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription;
        shaderResourceViewDescription.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        shaderResourceViewDescription.TextureCube.MipLevels = 1;
        shaderResourceViewDescription.TextureCube.MostDetailedMip = 0;

        m_Renderer->m_GraphicsDevice->m_Device->CreateShaderResourceView(textureResources.data()->Get(), &shaderResourceViewDescription, &m_ShaderResourceView);
        AURORA_WARNING("Created Cubemap SRV.");

        m_Renderer->LoadShader(Shader_Stage::Vertex_Shader, m_CubemapShaderVS, "SimpleCubemapVS.hlsl");
        m_Renderer->LoadShader(Shader_Stage::Pixel_Shader, m_CubemapShaderPS, "SimpleCubemapPS.hlsl");

        m_Renderer->LoadSkyPipelineState(&m_CubemapShaderVS, &m_CubemapShaderPS);
    }

    void Environment::Render()
    {
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->RSSetState(m_SkyRasterizerState);
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->OMSetDepthStencilState(m_SkyDepthState, 0);

        // Get Vertex Buffer from Cube
        // Get Index Buffer from Cube
        // Set View Matrix Shader
        // Set Projection Matrix Shader
        // Set Vertex Buffer
        // Set Index Buffer
        // Draw Indexed

        // Reset Changed Render States

    }
}