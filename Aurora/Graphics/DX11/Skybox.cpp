#include "Aurora.h"
#include "Skybox.h"
#include "../Scene/World.h"
#include "RenderTexture.h"
#include "../Resource/ResourceCache.h"
#include "../_Shaders/ShaderInternals.h"
#include "../Scene/Components/Transform.h"

namespace Aurora
{
    Skybox::Skybox(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_Renderer = m_EngineContext->GetSubsystem<Renderer>();
    }

    Skybox::~Skybox()
    {
    }

    ComPtr<ID3D11Buffer> Skybox::CreateConstantBuffer(const void* data, UINT size) const
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = static_cast<UINT>(size);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        D3D11_SUBRESOURCE_DATA bufferData = {};
        bufferData.pSysMem = data;

        ComPtr<ID3D11Buffer> buffer;
        const D3D11_SUBRESOURCE_DATA* bufferDataPtr = data ? &bufferData : nullptr;
        m_Renderer->m_GraphicsDevice->m_Device->CreateBuffer(&desc, bufferDataPtr, &buffer);
        
        return buffer;
    }

    MeshBuffer Skybox::CreateMeshBuffer(const std::shared_ptr<class MeshDerp>& mesh) const
    {
        MeshBuffer buffer = {};
        buffer.stride = sizeof(MeshDerp::Vertex);
        buffer.numElements = static_cast<UINT>(mesh->faces().size() * 3);

        const size_t vertexDataSize = mesh->vertices().size() * sizeof(MeshDerp::Vertex);
        const size_t indexDataSize = mesh->faces().size() * sizeof(MeshDerp::Face);

        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = (UINT)vertexDataSize;
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = &mesh->vertices()[0];
            m_Renderer->m_GraphicsDevice->m_Device->CreateBuffer(&desc, &data, &buffer.vertexBuffer);
        }

        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = (UINT)indexDataSize;
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = &mesh->faces()[0];
            m_Renderer->m_GraphicsDevice->m_Device->CreateBuffer(&desc, &data, &buffer.indexBuffer);
        }
        return buffer;
    }

    bool Skybox::InitializeResources()
    {

        // Create Cube Entity
        /*
        m_SkyboxEntity = m_EngineContext->GetSubsystem<World>()->CreateDefaultObject(DefaultObjectType::DefaultObjectType_Cube);
        m_SkyboxEntity->m_EntityType = EntityType::Skybox;
        m_SkyboxEntity->SetName("Skybox");

        for (int i = 0; i < m_SkyboxEntity->GetComponent<Transform>()->m_Children.size(); i++)
        {
            m_SkyboxEntity->GetComponent<Transform>()->m_Children[i]->GetEntity()->m_EntityType = EntityType::Skybox;
        }
        */
        const std::vector<D3D11_INPUT_ELEMENT_DESC> skyboxInputLayout = {
            { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        m_SkyboxEntity = CreateMeshBuffer(MeshDerp::fromFile("../Resources/Models/Skybox/skybox.obj"));

        // XMFLOAT3 rotationInDegrees = { 180, 0, 0 };
        // m_SkyboxEntity->m_Transform->m_RotationInRadians = { XMConvertToRadians(rotationInDegrees.x), XMConvertToRadians(rotationInDegrees.y), XMConvertToRadians(rotationInDegrees.z) };
        

        // ===============================================================
        ID3D11UnorderedAccessView* const nullUAV[] = { nullptr }; // Reset.

        D3D11_RASTERIZER_DESC rasterizerDescription;
        rasterizerDescription.FillMode = D3D11_FILL_SOLID;
        rasterizerDescription.CullMode = D3D11_CULL_BACK;
        rasterizerDescription.FrontCounterClockwise = true;
        rasterizerDescription.DepthClipEnable = true;
        m_Renderer->m_GraphicsDevice->m_Device->CreateRasterizerState(&rasterizerDescription, &m_DefaultRasterizerState);

        D3D11_DEPTH_STENCIL_DESC depthStencilDescription;
        depthStencilDescription.DepthEnable = true;
        depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS;
        m_Renderer->m_GraphicsDevice->m_Device->CreateDepthStencilState(&depthStencilDescription, &m_SkyboxDepthStencilState);

        m_DefaultSampler = CreateSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);
        m_ComputeSampler = CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);

        /// Shader & Input Layout for Skybox
        m_Renderer->LoadShader(Shader_Stage::Vertex_Shader, m_VSSkyboxShader, "SkyboxVS.hlsl");
        m_Renderer->LoadShader(Shader_Stage::Pixel_Shader,  m_PSSkyboxShader, "SkyboxPS.hlsl");

        DX11_Utility::DX11_VertexShaderPackage* vertexInternal = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_VSSkyboxShader.m_InternalState.get());
        m_Renderer->m_GraphicsDevice->m_Device->CreateInputLayout(skyboxInputLayout.data(), (UINT)skyboxInputLayout.size(), vertexInternal->m_ShaderCode.data(), vertexInternal->m_ShaderCode.size(), &m_InputLayout);

 
        // Unfiltered environment cubemap (temporary).
        Texture environmentTextureUnfiltered = CreateTextureCube(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);
        CreateTextureUAV(environmentTextureUnfiltered, 0);

        // Load and convert equirectangular environment map to a cubemap texture.
        {
            RHI_Shader equirectangularToCubeShader;
            m_Renderer->LoadShader(Shader_Stage::Compute_Shader, equirectangularToCubeShader, "CSEquirectangular.hlsl");
            ID3D11ComputeShader* shaderInternal = static_cast<DX11_Utility::DX11_ComputeShaderPackage*>(equirectangularToCubeShader.m_InternalState.get())->m_Resource.Get();

            m_EnvironmentTextureEquirectangular = CreateTexture(ImageDerp::fromFile("../Resources/Textures/HDR/environment.hdr"), DXGI_FORMAT_R32G32B32A32_FLOAT, 1);

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShaderResources(5, 1, m_EnvironmentTextureEquirectangular.m_SRV.GetAddressOf());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, environmentTextureUnfiltered.m_UAV.GetAddressOf(), nullptr);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetSamplers(3, 1, m_ComputeSampler.GetAddressOf());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShader(shaderInternal, nullptr, 0);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->Dispatch(environmentTextureUnfiltered.m_Width / 32, environmentTextureUnfiltered.m_Height / 32, 6);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
            AURORA_WARNING("Conversion to Equirectangle Map Success!");
        }

        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->GenerateMips(environmentTextureUnfiltered.m_SRV.Get());

        {
            struct SpecularMapFilterSettingsCB
            {
                float roughness;
                float padding[3];
            };

            ComPtr<ID3D11Buffer> spmapCB = CreateConstantBuffer<SpecularMapFilterSettingsCB>();

            // Compute pre-filtered specular environment map.

            RHI_Shader specularPrefilterMapShader;
            m_Renderer->LoadShader(Shader_Stage::Compute_Shader, specularPrefilterMapShader, "CSSpecularPrefilter.hlsl");
            ID3D11ComputeShader* shaderInternal = static_cast<DX11_Utility::DX11_ComputeShaderPackage*>(specularPrefilterMapShader.m_InternalState.get())->m_Resource.Get();

            m_EnvironmentTexture = CreateTextureCube(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);

            // Copy 0th mipmap level into destination environment map.
            for (int arraySlice = 0; arraySlice < 6; ++arraySlice)
            {
                const UINT subresourceIndex = D3D11CalcSubresource(0, arraySlice, m_EnvironmentTexture.m_Levels);
                m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CopySubresourceRegion(m_EnvironmentTexture.m_Texture.Get(), subresourceIndex, 0, 0, 0, environmentTextureUnfiltered.m_Texture.Get(), subresourceIndex, nullptr);
            }

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShaderResources(5, 1, environmentTextureUnfiltered.m_SRV.GetAddressOf());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetSamplers(3, 1, m_ComputeSampler.GetAddressOf());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShader(shaderInternal, nullptr, 0);

            // Pre-filter rest of the mip chain.
            const float deltaRoughness = 1.0 / XMMax(float(m_EnvironmentTexture.m_Levels - 1), 1.0f);
            for (UINT level = 1, size = 512; level < m_EnvironmentTexture.m_Levels; ++level, size /= 2)
            {
                const UINT numberOfGroups = XMMax<UINT>(1, size / 32);
                CreateTextureUAV(m_EnvironmentTexture, level);

                const SpecularMapFilterSettingsCB spmapConstants = { level * deltaRoughness };
                m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->UpdateSubresource(spmapCB.Get(), 0, nullptr, &spmapConstants, 0, 0);

                m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetConstantBuffers(7, 1, spmapCB.GetAddressOf());
                m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, m_EnvironmentTexture.m_UAV.GetAddressOf(), nullptr);
                m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->Dispatch(numberOfGroups, numberOfGroups, 6);
            }

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
            AURORA_WARNING("Specular Map Prefilter Success!");
        }

        // Compute Diffuse Irradiance Cubemap
        {
            RHI_Shader diffuseIrradianceShader;
            m_Renderer->LoadShader(Shader_Stage::Compute_Shader, diffuseIrradianceShader, "CSIrradianceMap.hlsl");
            ID3D11ComputeShader* shaderInternal = static_cast<DX11_Utility::DX11_ComputeShaderPackage*>(diffuseIrradianceShader.m_InternalState.get())->m_Resource.Get();

            m_IrradianceMapTexture = CreateTextureCube(32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT, 1);
            CreateTextureUAV(m_IrradianceMapTexture, 0);

            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShaderResources(5, 1, m_EnvironmentTexture.m_SRV.GetAddressOf());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetSamplers(3, 1, m_ComputeSampler.GetAddressOf());
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, m_IrradianceMapTexture.m_UAV.GetAddressOf(), nullptr);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetShader(shaderInternal, nullptr, 0);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->Dispatch(m_IrradianceMapTexture.m_Width / 32, m_IrradianceMapTexture.m_Height / 32, 6);
            m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
        }
        
        // Compute Cook-Torrace BRDF 2D LUT for split-sum approximation.
        {
            RHI_Shader specularPrefilterBRDFShader;
            m_Renderer->LoadShader(Shader_Stage::Compute_Shader, specularPrefilterBRDFShader, "CSSpecularPrefilterBRDF.hlsl");
            ID3D11ComputeShader* shaderInternal = static_cast<DX11_Utility::DX11_ComputeShaderPackage*>(specularPrefilterBRDFShader.m_InternalState.get())->m_Resource.Get();

            m_SpecularPrefilterBRDFLUT = CreateTexture(256, 256, DXGI_FORMAT_R16G16_FLOAT, 1);
            m_SpecularBRDFSampler = CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);
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
         // m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->RSSetState(m_DefaultRasterizerState.Get());
         
         // Mesh* meshComponent = m_SkyboxEntity->m_Transform->GetChildByIndex(0)->GetEntity()->GetComponent<Mesh>();

         // UINT offset = 0;
         // UINT modelStride = 8 * sizeof(float);

         // Update constant buffer data.
         // if (meshComponent->GetEntity()->HasComponent<Material>())
         // {
         //    m_Renderer->UpdateMaterialConstantBuffer(meshComponent->GetEntity()->GetComponent<Material>());
         // }

         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetInputLayout(m_InputLayout.Get());
         // ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&meshComponent->m_VertexBuffer_Position)->m_Resource.Get();
         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, m_SkyboxEntity.vertexBuffer.GetAddressOf(), &m_SkyboxEntity.stride, &m_SkyboxEntity.offset);
         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetIndexBuffer(m_SkyboxEntity.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

         ID3D11VertexShader* shaderInternalVS = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_VSSkyboxShader.m_InternalState.get())->m_Resource.Get();
         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(shaderInternalVS, nullptr, 0);
         ID3D11PixelShader* shaderInternalPS = static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_PSSkyboxShader.m_InternalState.get())->m_Resource.Get();
         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(shaderInternalPS, nullptr, 0);

         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(5, 1, m_EnvironmentTexture.m_SRV.GetAddressOf());
         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(3, 1, m_DefaultSampler.GetAddressOf());
         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(4, 1, m_SpecularBRDFSampler.GetAddressOf());
         // m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->OMSetDepthStencilState(m_SkyboxDepthStencilState.Get(), 0);
         m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->DrawIndexed(m_SkyboxEntity.numElements, 0, 0);

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


    ComPtr<ID3D11SamplerState> Skybox::CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) const
    {
        D3D11_SAMPLER_DESC samplerDescription = {};
        samplerDescription.Filter = filter;
        samplerDescription.AddressU = addressMode;
        samplerDescription.AddressV = addressMode;
        samplerDescription.AddressW = addressMode;
        samplerDescription.MaxAnisotropy = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
        samplerDescription.MinLOD = 0;
        samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

        ComPtr<ID3D11SamplerState> samplerState;
        m_Renderer->m_GraphicsDevice->m_Device->CreateSamplerState(&samplerDescription, &samplerState);

        return samplerState;
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