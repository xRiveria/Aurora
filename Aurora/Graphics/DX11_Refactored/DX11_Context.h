#pragma once
#include "../RHI_Implementation.h"
#include "../RHI_Utilities.h"
#include "../DX11_Refactored/DX11_VertexBuffer.h"


/*  == DX11 Context ==

    Second level of abstraction postceding the Renderer class. Once we begin to support further APIs, there will be a further abstraction for the Context class, which calls underlying APIs below it.    
    The context class serves as the brain of each graphics API supported by Aurora. It provides general commands such as the creation of resources, binding, drawing amongst others.

    All lower level work is propagated into the respective resource classes.

    As we are planning for future API additions, you will see bits of sprinkled abstraction comments for my future usage.
*/

namespace Aurora
{
    class DX11_ConstantBuffer;
    class DX11_VertexBuffer;
    class DX11_IndexBuffer;
    class DX11_InputLayout;
    class DX11_Sampler;

    class DX11_Context
    {
    public:
        DX11_Context(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext);

        template<typename T>
        inline std::shared_ptr<DX11_VertexBuffer> CreateVertexBuffer(RHI_Vertex_Type vertexType, std::vector<T>& vertices)
        {
            std::shared_ptr<DX11_VertexBuffer> vertexBuffer = std::make_shared<DX11_VertexBuffer>();
            vertexBuffer->Initialize(vertexType, vertices, m_Devices.get());
           
            return vertexBuffer;
        }
        void BindVertexBuffer(DX11_VertexBuffer* vertexBuffer);

        std::shared_ptr<DX11_IndexBuffer> CreateIndexBuffer(std::vector<uint32_t>& indices);
        void BindIndexBuffer(DX11_IndexBuffer* indexBuffer);

        std::shared_ptr<DX11_InputLayout> CreateInputLayout(RHI_Vertex_Type vertexType, std::vector<uint8_t>& vertexShaderBlob);
        void BindInputLayout(DX11_InputLayout* inputLayout);

        std::shared_ptr<DX11_ConstantBuffer> CreateConstantBuffer(const std::string& bufferName, uint32_t bufferSize);
        void* UpdateConstantBuffer(DX11_ConstantBuffer* constantBuffer, const void* bufferData);
        void BindConstantBuffer(RHI_Shader_Stage shaderStage, uint32_t slotNumber, uint32_t slotCount, DX11_ConstantBuffer* constantBuffer);

        // We make the simple assumption that the address mode and border color are the same across all of its respective values.
        std::shared_ptr<DX11_Sampler> CreateSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, D3D11_COMPARISON_FUNC comparisonFunction, float mipLODBias, float borderColor);
        void BindSampler(RHI_Shader_Stage shaderStage, uint32_t slotNumber, uint32_t slotCount, DX11_Sampler* sampler);

    private:
        std::shared_ptr<DX11_Devices> m_Devices;
    };
}