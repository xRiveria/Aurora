#include "Aurora.h"
#include "DX11_Context.h"
#include "DX11_VertexBuffer.h"
#include "DX11_IndexBuffer.h"
#include "DX11_ConstantBuffer.h"
#include "DX11_InputLayout.h"
#include "DX11_Sampler.h"
#include "DX11_RasterizerState.h"

namespace Aurora
{
    DX11_Context::DX11_Context(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext)
    {
        m_Devices = std::make_shared<DX11_Devices>();

        m_Devices->m_Device = device.Get();
        m_Devices->m_DeviceContextImmediate = deviceContext.Get();
    }

    void DX11_Context::Initialize()
    {
        CreateSwapchain();
        CreateRasterizerStates();
    }

    void DX11_Context::CreateSwapchain()
    {
        // Determine maximum supported MSAA level.
        QuerySupportedMultisamplingLevels(16);
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
        rasterizerStateDescription.AntialiasedLineEnable = false;
        rasterizerStateDescription.MultisampleEnable = false;
        rasterizerStateDescription.ScissorEnable = true;

        m_RasterizerStates[RasterizerState_Types::RasterizerState_Sky] = CreateRasterizerState(rasterizerStateDescription);

        rasterizerStateDescription.FillMode = D3D11_FILL_WIREFRAME;
        m_RasterizerStates[RasterizerState_Types::RasterizerState_Wireframe] = CreateRasterizerState(rasterizerStateDescription);

        rasterizerStateDescription.FillMode = D3D11_FILL_SOLID;
        rasterizerStateDescription.CullMode = D3D11_CULL_NONE;
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

    void DX11_Context::QuerySupportedMultisamplingLevels(uint32_t requestedLevels)
    {
        // Loop from the top divided by 2 each iteration. We select the highest possible sampling level we can supported.
        for (m_SupportedMultisamplingLevelCount = requestedLevels; m_SupportedMultisamplingLevelCount > 1; m_SupportedMultisamplingLevelCount /= 2)
        {
            UINT colorQualityLevels;
            UINT depthStencilQualityLevels;

            m_Devices->m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R16G16B16A16_FLOAT, m_SupportedMultisamplingLevelCount, &colorQualityLevels);
            m_Devices->m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, m_SupportedMultisamplingLevelCount, &depthStencilQualityLevels);

            if (colorQualityLevels > 0 && depthStencilQualityLevels > 0)
            {
                break;
            }
        }

        AURORA_INFO(LogLayer::Graphics, "Multisampling Level Supported by DX11 Adapter: %u", m_SupportedMultisamplingLevelCount);
    }
}