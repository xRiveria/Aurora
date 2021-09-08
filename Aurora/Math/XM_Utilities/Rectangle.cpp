#include "Aurora.h"
#include "Rectangle.h"
#include "../Renderer/Renderer.h"
#include "../Graphics/RHI_Vertex.h"

namespace Aurora::Math
{
    bool Rectangle::CreateBuffers(Renderer* renderer)
    {
        const float renderWidth = renderer->m_RenderWidth;
        const float renderHeight = renderer->m_RenderHeight;

        const float scLeft = -(renderWidth * 0.5f) + left;
        const float scRight = scLeft + Width();
        const float scTop = (renderHeight * 0.5f) - top;
        const float scBottom = scTop - Height();

        // Create Vertex Buffer
        std::vector<RHI_Vertex_Position_UV> vertices;
        {
            // First Triangle
            vertices.emplace_back(RHI_Vertex_Position_UV(XMFLOAT3(scLeft, scTop, 0.0f), XMFLOAT2(0.0f, 0.0f))); // Top left
            vertices.emplace_back(RHI_Vertex_Position_UV(XMFLOAT3(scRight, scBottom, 0.0f), XMFLOAT2(1.0f, 1.0f))); // Bottom right
            vertices.emplace_back(RHI_Vertex_Position_UV(XMFLOAT3(scLeft, scBottom, 0.0f), XMFLOAT2(0.0f, 1.0f))); // Bottom left

            // Second Triangle
            vertices.emplace_back(RHI_Vertex_Position_UV(XMFLOAT3(scLeft, scTop, 0.0f), XMFLOAT2(0.0f, 0.0f))); // Top left
            vertices.emplace_back(RHI_Vertex_Position_UV(XMFLOAT3(scRight, scTop, 0.0f), XMFLOAT2(1.0f, 0.0f))); // Top right
            vertices.emplace_back(RHI_Vertex_Position_UV(XMFLOAT3(scRight, scBottom, 0.0f), XMFLOAT2(1.0f, 1.0f)));  // Bottom right
        };

        m_VertexBuffer = renderer->m_DeviceContext->CreateVertexBuffer<RHI_Vertex_Position_UV>(RHI_Vertex_Type::VertexType_PositionUV, vertices);

        // Create Index Buffer
        std::vector<uint32_t> indices;
        indices.emplace_back(0);
        indices.emplace_back(1);
        indices.emplace_back(2);
        indices.emplace_back(3);
        indices.emplace_back(4);
        indices.emplace_back(5);

        m_IndexBuffer = renderer->m_DeviceContext->CreateIndexBuffer(indices);

        return true;
    }
}