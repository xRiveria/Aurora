#pragma once
#include "../RHI_Implementation.h"
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

        std::shared_ptr<DX11_IndexBuffer> CreateIndexBuffer(std::vector<uint32_t>& indices);

        bool CreateConstantBuffer(D3D11_BUFFER_DESC* bufferDescription, D3D11_SUBRESOURCE_DATA* initialBufferData);

    private:
        std::shared_ptr<DX11_Devices> m_Devices;
    };
}