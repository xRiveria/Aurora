#include "Aurora.h"
#include "DX11_VertexBuffer.h"
#include "DX11_IndexBuffer.h"

namespace Aurora
{
    DX11_IndexBuffer::~DX11_IndexBuffer()
    {
        _DestroyBuffer();
    }

    void DX11_IndexBuffer::_DestroyBuffer()
    {
        if (m_IndexBuffer != nullptr)
        {
            m_IndexBuffer.Reset();
            AURORA_INFO(LogLayer::Graphics, "Successfully released Index Buffer.");
        }
    }

    bool DX11_IndexBuffer::Initialize(std::vector<uint32_t>& indices, DX11_Devices* devices)
    {
        AURORA_ASSERT(devices != nullptr);

        m_Devices = devices;
        m_Stride = static_cast<uint32_t>(sizeof(uint32_t));
        m_IndexCount = static_cast<uint32_t>(indices.size());
        m_ObjectSizeGPU = static_cast<uint64_t>(m_IndexCount * m_Stride);

        bool isDynamic = indices.data() == nullptr;

        // If a previous buffer existed, destroy it.
        _DestroyBuffer();

        D3D11_BUFFER_DESC bufferDescription = {};
        bufferDescription.ByteWidth = static_cast<UINT>(m_ObjectSizeGPU);
        bufferDescription.Usage = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
        bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDescription.CPUAccessFlags = isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
        bufferDescription.MiscFlags = 0;
        bufferDescription.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA subresourceDataDescription = {};
        subresourceDataDescription.pSysMem = static_cast<void*>(indices.data());
        subresourceDataDescription.SysMemPitch = 0; // Not needed for buffers.
        subresourceDataDescription.SysMemSlicePitch = 0; // Not needed for buffers.
        
        const HRESULT result = m_Devices->m_Device->CreateBuffer(&bufferDescription, &subresourceDataDescription, (ID3D11Buffer**)m_IndexBuffer.GetAddressOf());

        if (FAILED(result))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Index Buffer.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Vertex Buffer - %f and %f", static_cast<float>(m_Stride), static_cast<float>(m_ObjectSizeGPU));
        return true;
    }

    void* DX11_IndexBuffer::Map()
    {
        AURORA_ASSERT(m_Devices != nullptr);
        AURORA_ASSERT(m_IndexBuffer != nullptr);

        D3D11_MAPPED_SUBRESOURCE mappedResourceDescription = {};
        m_Devices->m_DeviceContextImmediate->Map(m_IndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceDescription);

        return mappedResourceDescription.pData;
    }

    bool DX11_IndexBuffer::Unmap()
    {
        AURORA_ASSERT(m_Devices != nullptr);
        AURORA_ASSERT(m_IndexBuffer != nullptr);

        m_Devices->m_DeviceContextImmediate->Unmap(m_IndexBuffer.Get(), 0);

        return true;
    }
}