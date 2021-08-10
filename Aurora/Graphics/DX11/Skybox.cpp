#include "Aurora.h"
#include "Skybox.h"
#include "../Scene/World.h"
#include "../Resource/ResourceCache.h"
#include "../_Shaders/ShaderInternals.h"
#include "../Scene/Components/Transform.h"
#include "../DX11_Refactored/DX11_ConstantBuffer.h"

namespace Aurora
{
    Skybox::Skybox(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_Renderer = m_EngineContext->GetSubsystem<Renderer>();
    }

    Skybox::~Skybox()
    {
    }


    std::shared_ptr<MeshBuffer> Skybox::CreateMeshBuffer(const std::shared_ptr<class MeshDerp>& mesh) const
    {
        std::shared_ptr<MeshBuffer> buffer = std::make_shared<MeshBuffer>();
        // buffer->stride = sizeof(MeshDerp::Vertex);

        buffer->vertexBuffer = m_Renderer->m_DeviceContext->CreateVertexBuffer(RHI_Vertex_Type::VertexType_PositionUVNormal, mesh->m_Vertices);
        buffer->indexBuffer = m_Renderer->m_DeviceContext->CreateIndexBuffer(mesh->m_Indices);


        return buffer;
    }

    bool Skybox::InitializeResources()
    {
        m_SkyboxEntity = CreateMeshBuffer(MeshDerp::fromFile("../Resources/Models/Skybox/skybox.obj"));

        // ===============================================================
        ID3D11UnorderedAccessView* const nullUAV[] = { nullptr }; // Reset.

        D3D11_DEPTH_STENCIL_DESC depthStencilDescription;
        depthStencilDescription.DepthEnable = true;
        depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS;
        m_Renderer->m_GraphicsDevice->m_Device->CreateDepthStencilState(&depthStencilDescription, &m_SkyboxDepthStencilState);

        m_DefaultSampler = m_Renderer->m_DeviceContext->CreateSampler(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_NEVER, 0.0f, 1.0f);
        m_ComputeSampler = m_Renderer->m_DeviceContext->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_NEVER, 0.0f, 1.0f);

        /// Shader & Input Layout for Skybox
        m_Renderer->LoadShader(RHI_Shader_Stage::Vertex_Shader, m_VSSkyboxShader, "SkyboxVS.hlsl");
        m_Renderer->LoadShader(RHI_Shader_Stage::Pixel_Shader,  m_PSSkyboxShader, "SkyboxPS.hlsl");

        DX11_Utility::DX11_VertexShaderPackage* vertexInternal = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_VSSkyboxShader.m_InternalState.get());
        m_InputLayout = m_Renderer->m_DeviceContext->CreateInputLayout(RHI_Vertex_Type::VertexType_Position, vertexInternal->m_ShaderCode);

        // Unfiltered environment cubemap (temporary).
        Texture environmentTextureUnfiltered = CreateTextureCube(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);
        CreateTextureUAV(environmentTextureUnfiltered, 0);

        // Load and convert equirectangular environment map to a cubemap texture.
        {
            RHI_Shader equirectangularToCubeShader;
            m_Renderer->LoadShader(RHI_Shader_Stage::Compute_Shader, equirectangularToCubeShader, "CSEquirectangular.hlsl");
            ID3D11ComputeShader* shaderInternal = static_cast<DX11_Utility::DX11_ComputeShaderPackage*>(equirectangularToCubeShader.m_InternalState.get())->m_Resource.Get();

            m_EnvironmentTextureEquirectangular = CreateTexture(ImageDerp::fromFile("../Resources/Textures/HDR/Night.hdr"), DXGI_FORMAT_R32G32B32A32_FLOAT, 1);

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShaderResources(5, 1, m_EnvironmentTextureEquirectangular.m_SRV.GetAddressOf());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, environmentTextureUnfiltered.m_UAV.GetAddressOf(), nullptr);
            m_Renderer->m_DeviceContext->BindSampler(RHI_Shader_Stage::Compute_Shader, 3, 1, m_ComputeSampler.get());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShader(shaderInternal, nullptr, 0);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->Dispatch(environmentTextureUnfiltered.m_Width / 32, environmentTextureUnfiltered.m_Height / 32, 6);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
            AURORA_WARNING(LogLayer::Graphics, "Conversion to Equirectangle Map Success!");
        }

        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->GenerateMips(environmentTextureUnfiltered.m_SRV.Get());

        {
            struct SpecularMapFilterSettingsCB
            {
                float roughness;
                float padding[3];
            };

            std::shared_ptr<DX11_ConstantBuffer> constantBuffer = m_Renderer->m_DeviceContext->CreateConstantBuffer("Specular Map Thingy", sizeof(SpecularMapFilterSettingsCB));

            // Compute pre-filtered specular environment map.

            RHI_Shader specularPrefilterMapShader;
            m_Renderer->LoadShader(RHI_Shader_Stage::Compute_Shader, specularPrefilterMapShader, "CSSpecularPrefilter.hlsl");
            ID3D11ComputeShader* shaderInternal = static_cast<DX11_Utility::DX11_ComputeShaderPackage*>(specularPrefilterMapShader.m_InternalState.get())->m_Resource.Get();

            m_EnvironmentTexture = CreateTextureCube(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);

            // Copy 0th mipmap level into destination environment map.
            for (int arraySlice = 0; arraySlice < 6; ++arraySlice)
            {
                const UINT subresourceIndex = D3D11CalcSubresource(0, arraySlice, m_EnvironmentTexture.m_Levels);
                m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CopySubresourceRegion(m_EnvironmentTexture.m_Texture.Get(), subresourceIndex, 0, 0, 0, environmentTextureUnfiltered.m_Texture.Get(), subresourceIndex, nullptr);
            }

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShaderResources(5, 1, environmentTextureUnfiltered.m_SRV.GetAddressOf());
            m_Renderer->m_DeviceContext->BindSampler(RHI_Shader_Stage::Compute_Shader, 3, 1, m_ComputeSampler.get());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShader(shaderInternal, nullptr, 0);

            // Pre-filter rest of the mip chain.
            const float deltaRoughness = 1.0 / XMMax(float(m_EnvironmentTexture.m_Levels - 1), 1.0f);
            for (UINT level = 1, size = 512; level < m_EnvironmentTexture.m_Levels; ++level, size /= 2)
            {
                const UINT numberOfGroups = XMMax<UINT>(1, size / 32);
                CreateTextureUAV(m_EnvironmentTexture, level);

                const SpecularMapFilterSettingsCB spmapConstants = { level * deltaRoughness };
                m_Renderer->m_DeviceContext->UpdateConstantBuffer(constantBuffer.get(), &spmapConstants);

                m_Renderer->m_DeviceContext->BindConstantBuffer(RHI_Shader_Stage::Compute_Shader, 7, 1, constantBuffer.get());

                m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, m_EnvironmentTexture.m_UAV.GetAddressOf(), nullptr);
                m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->Dispatch(numberOfGroups, numberOfGroups, 6);
            }

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
            AURORA_WARNING(LogLayer::Graphics, "Specular Map Prefilter Success!");
        }

        // Compute Diffuse Irradiance Cubemap
        {
            RHI_Shader diffuseIrradianceShader;
            m_Renderer->LoadShader(RHI_Shader_Stage::Compute_Shader, diffuseIrradianceShader, "CSIrradianceMap.hlsl");
            ID3D11ComputeShader* shaderInternal = static_cast<DX11_Utility::DX11_ComputeShaderPackage*>(diffuseIrradianceShader.m_InternalState.get())->m_Resource.Get();

            m_IrradianceMapTexture = CreateTextureCube(32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT, 1);
            CreateTextureUAV(m_IrradianceMapTexture, 0);

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShaderResources(5, 1, m_EnvironmentTexture.m_SRV.GetAddressOf());
            m_Renderer->m_DeviceContext->BindSampler(RHI_Shader_Stage::Compute_Shader, 3, 1, m_ComputeSampler.get());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, m_IrradianceMapTexture.m_UAV.GetAddressOf(), nullptr);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShader(shaderInternal, nullptr, 0);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->Dispatch(m_IrradianceMapTexture.m_Width / 32, m_IrradianceMapTexture.m_Height / 32, 6);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
        }
        
        // Compute Cook-Torrace BRDF 2D LUT for split-sum approximation.
        {
            RHI_Shader specularPrefilterBRDFShader;
            m_Renderer->LoadShader(RHI_Shader_Stage::Compute_Shader, specularPrefilterBRDFShader, "CSSpecularPrefilterBRDF.hlsl");
            ID3D11ComputeShader* shaderInternal = static_cast<DX11_Utility::DX11_ComputeShaderPackage*>(specularPrefilterBRDFShader.m_InternalState.get())->m_Resource.Get();

            m_SpecularPrefilterBRDFLUT = CreateTexture(256, 256, DXGI_FORMAT_R16G16_FLOAT, 1);
            m_SpecularBRDFSampler = m_Renderer->m_DeviceContext->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_COMPARISON_NEVER, 0.0, 1.0);
            CreateTextureUAV(m_SpecularPrefilterBRDFLUT, 0);

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, m_SpecularPrefilterBRDFLUT.m_UAV.GetAddressOf(), nullptr);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShader(shaderInternal, nullptr, 0);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->Dispatch(m_SpecularPrefilterBRDFLUT.m_Width / 32, m_SpecularPrefilterBRDFLUT.m_Height / 32, 1);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
        }
        
        return true;
    }

    bool Skybox::Render() const
    {
        // m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_Renderer->m_DeviceContext->BindRasterizerState(RasterizerState_Types::RasterizerState_Sky);
        m_Renderer->m_DeviceContext->BindInputLayout(m_InputLayout.get());
        m_Renderer->m_DeviceContext->BindVertexBuffer(m_SkyboxEntity->vertexBuffer.get());
        m_Renderer->m_DeviceContext->BindIndexBuffer(m_SkyboxEntity->indexBuffer.get());

        ID3D11VertexShader* shaderInternalVS = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_VSSkyboxShader.m_InternalState.get())->m_Resource.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(shaderInternalVS, nullptr, 0);
        ID3D11PixelShader* shaderInternalPS = static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_PSSkyboxShader.m_InternalState.get())->m_Resource.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(shaderInternalPS, nullptr, 0);

        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(5, 1, m_EnvironmentTexture.m_SRV.GetAddressOf());
        m_Renderer->m_DeviceContext->BindSampler(RHI_Shader_Stage::Pixel_Shader, 3, 1, m_DefaultSampler.get());
        m_Renderer->m_DeviceContext->BindSampler(RHI_Shader_Stage::Pixel_Shader, 4, 1, m_SpecularBRDFSampler.get());
        // m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->OMSetDepthStencilState(m_SkyboxDepthStencilState.Get(), 0);
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->DrawIndexed(m_SkyboxEntity->indexBuffer->GetIndexCount(), 0, 0);

        return true;
    }

    template<typename T>
    static constexpr T NumberOfMipmapLevels(T width, T height)
    {
        T levels = 1;
        while ((width | height) >> levels)
        {
            ++levels;
        }

        return levels;
    }

    Texture Skybox::CreateTextureCube(UINT width, UINT height, DXGI_FORMAT format, UINT levels) const
    {
        Texture texture;
        texture.m_Width = width;
        texture.m_Height = height;
        texture.m_Levels = (levels > 0) ? levels : NumberOfMipmapLevels(width, height);

        D3D11_TEXTURE2D_DESC textureDescription = {};
        textureDescription.Width = width;
        textureDescription.Height = height;
        textureDescription.MipLevels = levels;
        textureDescription.ArraySize = 6;
        textureDescription.Format = format;
        textureDescription.SampleDesc.Count = 1;
        textureDescription.Usage = D3D11_USAGE_DEFAULT;
        textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        textureDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        if (levels == 0)
        {
            textureDescription.BindFlags |= D3D11_BIND_RENDER_TARGET;
            textureDescription.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }

        m_Renderer->m_GraphicsDevice->m_Device->CreateTexture2D(&textureDescription, nullptr, &texture.m_Texture);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDescription;
        srvDescription.Format = textureDescription.Format;
        srvDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDescription.TextureCube.MostDetailedMip = 0;
        srvDescription.TextureCube.MipLevels = -1;

        m_Renderer->m_GraphicsDevice->m_Device->CreateShaderResourceView(texture.m_Texture.Get(), &srvDescription, &texture.m_SRV);

        return texture;
    }

    Texture Skybox::CreateTexture(UINT width, UINT height, DXGI_FORMAT format, UINT levels) const
    {
        Texture texture;
        texture.m_Width = width;
        texture.m_Height = height;
        texture.m_Levels = (levels > 0) ? levels : NumberOfMipmapLevels(width, height);
        
        D3D11_TEXTURE2D_DESC textureDescription = {};
        textureDescription.Width = width;
        textureDescription.Height = height;
        textureDescription.MipLevels = levels;
        textureDescription.ArraySize = 1;
        textureDescription.Format = format;
        textureDescription.SampleDesc.Count = 1;
        textureDescription.Usage = D3D11_USAGE_DEFAULT;
        textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

        if (levels == 0)
        {
            textureDescription.BindFlags |= D3D11_BIND_RENDER_TARGET;
            textureDescription.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }

        m_Renderer->m_GraphicsDevice->m_Device->CreateTexture2D(&textureDescription, nullptr, &texture.m_Texture);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
        srvDescription.Format = textureDescription.Format;
        srvDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDescription.Texture2D.MostDetailedMip = 0;
        srvDescription.Texture2D.MipLevels = -1;
        
        m_Renderer->m_GraphicsDevice->m_Device->CreateShaderResourceView(texture.m_Texture.Get(), &srvDescription, &texture.m_SRV);

        return texture;
    }

    Texture Skybox::CreateTexture(const std::shared_ptr<ImageDerp>& image, DXGI_FORMAT format, UINT levels) const
    {
        Texture texture = CreateTexture(image->width(), image->height(), format, levels);
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->UpdateSubresource(texture.m_Texture.Get(), 0, nullptr, image->pixels<void>(), image->pitch(), 0);
        if (levels == 0) {
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->GenerateMips(texture.m_SRV.Get());
        }
        return texture;
    }

    void Skybox::CreateTextureUAV(Texture& texture, UINT mipSlice) const
    {
        D3D11_TEXTURE2D_DESC textureDescription;
        texture.m_Texture->GetDesc(&textureDescription);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDescription = {};
        uavDescription.Format = textureDescription.Format;

        if (textureDescription.ArraySize == 1)
        {
            uavDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDescription.Texture2D.MipSlice = mipSlice;
        }
        else
        {
            uavDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDescription.Texture2DArray.MipSlice = mipSlice;
            uavDescription.Texture2DArray.FirstArraySlice = 0;
            uavDescription.Texture2DArray.ArraySize = textureDescription.ArraySize;
        }

        m_Renderer->m_GraphicsDevice->m_Device->CreateUnorderedAccessView(texture.m_Texture.Get(), &uavDescription, &texture.m_UAV);
    }



    
}