#include "Aurora.h"
#include "DX11_Context.h"
#include "DX11_VertexBuffer.h"
#include "DX11_IndexBuffer.h"
#include "DX11_ConstantBuffer.h"
#include "DX11_InputLayout.h"
#include "DX11_Sampler.h"
#include "DX11_RasterizerState.h"
#include "DX11_Texture.h"

namespace Aurora
{
    DX11_Context::DX11_Context(EngineContext* engineContext, ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext)
    {
        m_Devices = std::make_shared<DX11_Devices>();

        m_Devices->m_Device = device.Get();
        m_Devices->m_DeviceContextImmediate = deviceContext.Get();
        m_EngineContext = engineContext;
    }

    void DX11_Context::Initialize()
    {
        CreateSwapchain();
        CreateRasterizerStates();
    }

    void DX11_Context::CreateSwapchain()
    {
        // Determine maximum supported MSAA level.
        SetMultisampleLevel(QuerySupportedMultisamplingLevels(16));
        ResizeBuffers();
    }

    void DX11_Context::ResizeBuffers()
    {
        AURORA_INFO(LogLayer::Serialization, "Resizing Buffers...");

        // Multisample Framebuffer
        m_MultisampleFramebuffer = std::make_shared<DX11_Framebuffer>(m_EngineContext);
        m_MultisampleFramebuffer->m_RenderTargetTexture->Initialize2DTexture(m_RenderWidth, m_RenderHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, DX11_ResourceViewFlag::Texture_Flag_RTV, m_Devices.get(), m_CurrentMultisampleLevelCount, 1);
        m_MultisampleFramebuffer->m_DepthStencilTexture->Initialize2DTexture(m_RenderWidth, m_RenderHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, DX11_ResourceViewFlag::Texture_Flag_DSV, m_Devices.get(), m_CurrentMultisampleLevelCount, 1);
        AURORA_INFO(LogLayer::Serialization, "Resized MSAA Framebuffer.");

        // Resolve Framebuffer
        m_ResolveFramebuffer = std::make_shared<DX11_Framebuffer>(m_EngineContext);
        m_ResolveFramebuffer->m_RenderTargetTexture->Initialize2DTexture(m_RenderWidth, m_RenderHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, DX11_ResourceViewFlag::Texture_Flag_RTV | DX11_ResourceViewFlag::Texture_Flag_SRV, m_Devices.get(), 1);
        AURORA_INFO(LogLayer::Serialization, "Resized Resolve Framebuffer.");

        // Bloom Texture
        m_BloomRenderTexture = CreateTexture2D(m_RenderWidth, m_RenderHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, DX11_ResourceViewFlag::Texture_Flag_RTV | DX11_ResourceViewFlag::Texture_Flag_SRV, 1, 1, 0);
        m_DummyDepthTexture = CreateTexture2D(m_RenderWidth, m_RenderHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, DX11_ResourceViewFlag::Texture_Flag_DSV, 1);

        // Shadow Depth Texture
        m_ShadowDepthTexture = CreateTexture2D(m_RenderWidth, m_RenderHeight, DXGI_FORMAT_R32G8X24_TYPELESS, DX11_ResourceViewFlag::Texture_Flag_DSV | DX11_ResourceViewFlag::Texture_Flag_SRV, 1, 1, 0);

        AURORA_INFO(LogLayer::Serialization, "Resized Bloom Render Target.");

        AURORA_INFO(LogLayer::Serialization, "Buffers Resized!");
    }

    void DX11_Context::CreateRasterizerStates()
    {
        D3D11_RASTERIZER_DESC rasterizerStateDescription = {};
        rasterizerStateDescription.FillMode = D3D11_FILL_SOLID;
        rasterizerStateDescription.CullMode = D3D11_CULL_BACK;
        rasterizerStateDescription.FrontCounterClockwise = true;
        rasterizerStateDescription.DepthClipEnable = true;
        rasterizerStateDescription.DepthBias = 0;
        rasterizerStateDescription.SlopeScaledDepthBias = 0.0f;
        rasterizerStateDescription.DepthBiasClamp = 0.0f;
        rasterizerStateDescription.AntialiasedLineEnable = false; // Has no effect on points and triangles with MSAA and impacts only selection of line-rendering algorithm (post feature level 10.1).
        rasterizerStateDescription.MultisampleEnable = false;
        rasterizerStateDescription.ScissorEnable = true;

        m_RasterizerStates[RasterizerState_Types::RasterizerState_Sky] = CreateRasterizerState(rasterizerStateDescription);

        rasterizerStateDescription.FillMode = D3D11_FILL_WIREFRAME;
        m_RasterizerStates[RasterizerState_Types::RasterizerState_Wireframe] = CreateRasterizerState(rasterizerStateDescription);

        rasterizerStateDescription.AntialiasedLineEnable = true;
        m_RasterizerStates[RasterizerState_Types::RasterizerState_CullBackWireframe] = CreateRasterizerState(rasterizerStateDescription);

        rasterizerStateDescription.FillMode = D3D11_FILL_SOLID;
        rasterizerStateDescription.CullMode = D3D11_CULL_NONE;
        rasterizerStateDescription.AntialiasedLineEnable = false;
        m_RasterizerStates[RasterizerState_Types::RasterizerState_Shadow] = CreateRasterizerState(rasterizerStateDescription);
    }

    void DX11_Context::BindVertexBuffer(DX11_VertexBuffer* vertexBuffer)
    {
        uint32_t bufferStride = vertexBuffer->GetStride();
        uint32_t bufferOffset = vertexBuffer->GetOffset();
        m_Devices->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, (ID3D11Buffer**)vertexBuffer->GetVertexBuffer().GetAddressOf(), (UINT*)(&bufferStride), (UINT*)(&bufferOffset));
    }

    std::shared_ptr<DX11_IndexBuffer> DX11_Context::CreateIndexBuffer(std::vector<uint32_t>& indices)
    {
        std::shared_ptr<DX11_IndexBuffer> indexBuffer = std::make_shared<DX11_IndexBuffer>();
        indexBuffer->Initialize(indices, m_Devices.get());
        return indexBuffer;
    }

    void DX11_Context::BindIndexBuffer(DX11_IndexBuffer* indexBuffer)
    {
        m_Devices->m_DeviceContextImmediate->IASetIndexBuffer(indexBuffer->GetIndexBuffer(), indexBuffer->Is16Bit() ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, (UINT)indexBuffer->GetOffset());
    }

    std::shared_ptr<DX11_InputLayout> DX11_Context::CreateInputLayout(RHI_Vertex_Type vertexType, std::vector<uint8_t>& vertexShaderBlob)
    {
        std::shared_ptr<DX11_InputLayout> inputLayout = std::make_shared<DX11_InputLayout>();
        inputLayout->Initialize(vertexType, vertexShaderBlob, m_Devices.get());
        return inputLayout;
    }

    void DX11_Context::BindInputLayout(DX11_InputLayout* inputLayout)
    {
        m_Devices->m_DeviceContextImmediate->IASetInputLayout(inputLayout->GetInputLayout().Get());
    }

    std::shared_ptr<DX11_ConstantBuffer> DX11_Context::CreateConstantBuffer(const std::string& bufferName, uint32_t bufferSize)
    {
        std::shared_ptr<DX11_ConstantBuffer> constantBuffer = std::make_shared<DX11_ConstantBuffer>();
        constantBuffer->Initialize(bufferName, bufferSize, m_Devices.get());
        return constantBuffer;
    }

    void* DX11_Context::UpdateConstantBuffer(DX11_ConstantBuffer* constantBuffer, const void* bufferData)
    {
        if (constantBuffer->GetConstantBuffer() == nullptr || constantBuffer->GetBufferSize() == 0)
        {
            AURORA_ERROR(LogLayer::Graphics, "Constant Buffer \"%s\" is invalid.", constantBuffer->GetObjectName().c_str());
        }

        void* bufferPointer = constantBuffer->Map();
        memcpy(bufferPointer, bufferData, constantBuffer->GetBufferSize());
        constantBuffer->Unmap();

        return nullptr;
    }

    void DX11_Context::BindConstantBuffer(RHI_Shader_Stage shaderStage, uint32_t slotNumber, uint32_t slotCount, DX11_ConstantBuffer* constantBuffer)
    {
        AURORA_ASSERT(constantBuffer->GetConstantBuffer() != nullptr);

        switch (shaderStage)
        {
            case RHI_Shader_Stage::Vertex_Shader:
                m_Devices->m_DeviceContextImmediate->VSSetConstantBuffers(slotNumber, slotCount, (ID3D11Buffer**)constantBuffer->GetConstantBuffer().GetAddressOf());
                break;

            case RHI_Shader_Stage::Pixel_Shader:
                m_Devices->m_DeviceContextImmediate->PSSetConstantBuffers(slotNumber, slotCount, (ID3D11Buffer**)constantBuffer->GetConstantBuffer().GetAddressOf());
                break;

            case RHI_Shader_Stage::Compute_Shader:
                m_Devices->m_DeviceContextImmediate->CSSetConstantBuffers(slotNumber, slotCount, (ID3D11Buffer**)constantBuffer->GetConstantBuffer().GetAddressOf());
                break;
        }
    }

    std::shared_ptr<DX11_Texture> DX11_Context::CreateTexture2D(uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, uint32_t textureFlags, uint32_t sampleLevels, uint32_t mipLevels, uint32_t mipSlice)
    {
        std::shared_ptr<DX11_Texture> texture = std::make_shared<DX11_Texture>(m_EngineContext);
        texture->Initialize2DTexture(textureWidth, textureHeight, format, textureFlags, m_Devices.get(), sampleLevels, mipLevels, mipSlice);
        return texture;
    }

    std::shared_ptr<DX11_Texture> DX11_Context::CreateTexture2DFromData(const void* sourceData, uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, uint32_t sampleLevels, uint32_t textureFlags, uint32_t mipSlice)
    {
        std::shared_ptr<DX11_Texture> texture = std::make_shared<DX11_Texture>(m_EngineContext);
        texture->Initialize2DTextureFromFile(sourceData, textureWidth, textureHeight, format, sampleLevels, textureFlags, mipSlice, m_Devices.get());
        return texture;
    }

    std::shared_ptr<DX11_Texture> DX11_Context::CreateTextureCube(uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, uint32_t mipLevels)
    {
        std::shared_ptr<DX11_Texture> textureCube = std::make_shared<DX11_Texture>(m_EngineContext);
        textureCube->InitializeTextureCube(textureWidth, textureHeight, format, m_Devices.get(), mipLevels);
        return textureCube;
    }

    std::shared_ptr<DX11_Sampler> DX11_Context::CreateSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, D3D11_COMPARISON_FUNC comparisonFunction, float mipLODBias, float borderColor)
    {
        std::shared_ptr<DX11_Sampler> samplerState = std::make_shared<DX11_Sampler>();
        samplerState->Initialize(filter, addressMode, comparisonFunction, mipLODBias, borderColor, m_Devices.get());
        return samplerState;
    }

    void DX11_Context::BindSampler(RHI_Shader_Stage shaderStage, uint32_t slotNumber, uint32_t slotCount, DX11_Sampler* sampler)
    {
        AURORA_ASSERT(sampler->GetSampler() != nullptr);

        switch (shaderStage)
        {
            case RHI_Shader_Stage::Vertex_Shader:
                m_Devices->m_DeviceContextImmediate->VSSetSamplers(slotNumber, slotCount, sampler->GetSampler().GetAddressOf());
                break;

            case RHI_Shader_Stage::Pixel_Shader:
                m_Devices->m_DeviceContextImmediate->PSSetSamplers(slotNumber, slotCount, sampler->GetSampler().GetAddressOf());
                break;

            case RHI_Shader_Stage::Compute_Shader:
                m_Devices->m_DeviceContextImmediate->CSSetSamplers(slotNumber, slotCount, sampler->GetSampler().GetAddressOf());
                break;
        }
    }

    std::shared_ptr<DX11_RasterizerState> DX11_Context::CreateRasterizerState(D3D11_RASTERIZER_DESC& rasterizerStateDescription)
    {
        std::shared_ptr<DX11_RasterizerState> rasterizerState = std::make_shared<DX11_RasterizerState>();
        rasterizerState->Initialize(rasterizerStateDescription, m_Devices.get());
        return rasterizerState;
    }

    void DX11_Context::BindRasterizerState(RasterizerState_Types rasterizerState) const
    {
        m_Devices->m_DeviceContextImmediate->RSSetState(m_RasterizerStates[rasterizerState]->GetRasterizerState().Get());
    }

    void DX11_Context::BindPrimitiveTopology(RHI_Primitive_Topology topology) const
    {
        switch (topology)
        {
            case RHI_Primitive_Topology::PointList:
                m_Devices->m_DeviceContextImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
                break;
            case RHI_Primitive_Topology::LineList:
                m_Devices->m_DeviceContextImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
                break;
            case RHI_Primitive_Topology::LineStrip:
                m_Devices->m_DeviceContextImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
                break;
            case RHI_Primitive_Topology::TriangleList:
                m_Devices->m_DeviceContextImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                break;
            case RHI_Primitive_Topology::TriangleStrip:
                m_Devices->m_DeviceContextImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                break;
            case RHI_Primitive_Topology::Undefined:
                AURORA_WARNING(LogLayer::Graphics, "Selected Primitive Topology type is undefined.");
                break;
        }
    }

    void DX11_Context::SetMultisampleLevel(uint32_t multisampleLevel)
    {
        if (multisampleLevel > m_MaxSupportedMultisamplingLevelCount || 0 > multisampleLevel)
        {
            AURORA_ERROR(LogLayer::Graphics, "Error setting multisample level to %u", multisampleLevel);
            return;
        }

        if (multisampleLevel == m_CurrentMultisampleLevelCount) { return; }

        m_CurrentMultisampleLevelCount = multisampleLevel;

        // Resize textures afterwards.
        AURORA_INFO(LogLayer::Graphics, "Successfully set multisample level to %u.", multisampleLevel);
    }

    void DX11_Context::ResolveFramebuffer(const DX11_Framebuffer* sourceFramebuffer, const DX11_Framebuffer* destinationFramebuffer, DXGI_FORMAT format)
    {
        if (sourceFramebuffer->m_RenderTargetTexture->GetTexture() != destinationFramebuffer->m_RenderTargetTexture->GetTexture())
        {
            m_Devices->m_DeviceContextImmediate->ResolveSubresource(destinationFramebuffer->m_RenderTargetTexture->GetTexture().Get(), 0, sourceFramebuffer->m_RenderTargetTexture->GetTexture().Get(), 0, format);
        }
    }

    uint32_t DX11_Context::QuerySupportedMultisamplingLevels(uint32_t requestedLevels)
    {
        // Hardwares must suppor 1, 4 and 8 sample counts by default. More can be exposed by vendors naturally. We select the highest possible sampling level we can support.
        // Standard sample patterns are 1 (trivial), 2, 4, 8 and 16.
        for (m_MaxSupportedMultisamplingLevelCount = requestedLevels; m_MaxSupportedMultisamplingLevelCount > 1; m_MaxSupportedMultisamplingLevelCount /= 2)
        {
            UINT colorQualityLevels;
            UINT depthStencilQualityLevels;
            
            // CheckMultisampleQualityLevels will return 0 if the given format and level count isn't supported by the adapter. Hence, we will pick the highest level returned.
            m_Devices->m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R16G16B16A16_FLOAT, m_MaxSupportedMultisamplingLevelCount, &colorQualityLevels);
            m_Devices->m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, m_MaxSupportedMultisamplingLevelCount, &depthStencilQualityLevels);

            if (colorQualityLevels > 0 && depthStencilQualityLevels > 0)
            {
                m_MaxSupportedMultisamplingQualityCount = colorQualityLevels; // 1
                break;
            }
        }

        AURORA_INFO(LogLayer::Graphics, "Multisampling Level Supported by DX11 Adapter: %u", m_MaxSupportedMultisamplingLevelCount);
        return m_MaxSupportedMultisamplingLevelCount;
    }
}