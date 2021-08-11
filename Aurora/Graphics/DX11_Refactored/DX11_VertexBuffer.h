#pragma once
#include "../Resource/AuroraObject.h"
#include "../RHI_Implementation.h"
#include "../RHI_Vertex.h"

namespace Aurora
{
    class DX11_VertexBuffer : public AuroraObject
    {
    public:
        DX11_VertexBuffer() = default;
        ~DX11_VertexBuffer();

        template<typename T>
        bool Initialize(RHI_Vertex_Type vertexType, std::vector<T>& vertices, DX11_Devices* devices);
        void* Map();
        bool Unmap();

        uint32_t GetStride() const { return m_Stride; }
        uint32_t GetOffset() const { return m_Offset; }
        uint32_t GetVertexCount() const { return m_VertexCount; }
        ComPtr<ID3D11Resource> GetVertexBuffer() const { return m_VertexBuffer; }

    private:
        void _DestroyBuffer();

    private:
        uint32_t m_Stride = 0;
        uint32_t m_VertexCount = 0;
        uint32_t m_Offset = 0;
        RHI_Vertex_Type m_VertexType = RHI_Vertex_Type::VertexType_Unknown;

        // API Specific
        ComPtr<ID3D11Resource> m_VertexBuffer = nullptr;
        DX11_Devices* m_Devices = nullptr;
    };
}