#include "Aurora.h"
#include "DX11_VertexBuffer.h"

namespace Aurora
{
    DX11_VertexBuffer::~DX11_VertexBuffer()
    {
        _DestroyBuffer();
    }

    void DX11_VertexBuffer::_DestroyBuffer()
    {
        if (m_VertexBuffer != nullptr)
        {
            m_VertexBuffer.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully released Vertex Buffer.");
        }
    }

    template<typename T>
    bool DX11_VertexBuffer::Initialize(RHI_Vertex_Type vertexType, std::vector<T>& vertices, DX11_Devices* devices)
    {
        AURORA_ASSERT(devices != nullptr);

        m_Devices = devices;
        m_VertexType = vertexType;
        m_Stride = static_cast<uint32_t>(sizeof(T));
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        m_ObjectSizeGPU = static_cast<uint64_t>(m_VertexCount * m_Stride); 

        // If we create the buffer without initial data, we expect to append to its data at some point later in time through mapping. Hence, it is dynamic.
        bool isDynamicBuffer = vertices.data() == nullptr;

        // If a previous buffer existed, destroy it.
        _DestroyBuffer();

        D3D11_BUFFER_DESC bufferDescription = {};
        bufferDescription.ByteWidth = static_cast<UINT>(m_ObjectSizeGPU);
        bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDescription.MiscFlags = 0;
        bufferDescription.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA subresourceDataDescription = {};
        subresourceDataDescription.pSysMem = static_cast<const void*>(vertices.data());
        subresourceDataDescription.SysMemPitch = 0; // No use in buffers.
        subresourceDataDescription.SysMemSlicePitch = 0; // No use in buffers.

        const HRESULT result = m_Devices->m_Device->CreateBuffer(&bufferDescription, isDynamicBuffer ? nullptr : &subresourceDataDescription, (ID3D11Buffer**)m_VertexBuffer.GetAddressOf());

        if (FAILED(result))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Vertex Buffer.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Vertex Buffer - %f and %f", static_cast<float>(m_Stride), static_cast<float>(m_ObjectSizeGPU));
        return true;
    }

    void* DX11_VertexBuffer::Map()
    {
        AURORA_ASSERT(m_Devices != nullptr);
        AURORA_ASSERT(m_VertexBuffer != nullptr);

        // Diable GPU access to the vertex buffer data. Upon mapping completion, use memcpy() to continue.
        D3D11_MAPPED_SUBRESOURCE mappedResourceDescription = {};
        const HRESULT result = m_Devices->m_DeviceContextImmediate->Map(m_VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceDescription);

        if (FAILED(result))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to map buffer resource.");
            return nullptr;
        }

        return mappedResourceDescription.pData;
    }

    bool DX11_VertexBuffer::Unmap()
    {
        AURORA_ASSERT(m_Devices != nullptr);
        AURORA_ASSERT(m_VertexBuffer != nullptr);

        // Re-enable GPU access to the vertex buffer data.
        m_Devices->m_DeviceContextImmediate->Unmap(m_VertexBuffer.Get(), 0);

        return false;
    }

    template bool DX11_VertexBuffer::Initialize(RHI_Vertex_Type vertexType, std::vector<RHI_Vertex_Position>& vertices, DX11_Devices* devices);
    template bool DX11_VertexBuffer::Initialize(RHI_Vertex_Type vertexType, std::vector<RHI_Vertex_Position_UV>& vertices, DX11_Devices* devices);
    template bool DX11_VertexBuffer::Initialize(RHI_Vertex_Type vertexType, std::vector<RHI_Vertex_Position_Color>& vertices, DX11_Devices* devices);
    template bool DX11_VertexBuffer::Initialize(RHI_Vertex_Type vertexType, std::vector<RHI_Vertex_Position_UV_Normal>& vertices, DX11_Devices* devices);
}