#include "Aurora.h"
#include "DX11_ConstantBuffer.h"

namespace Aurora
{
    DX11_ConstantBuffer::~DX11_ConstantBuffer()
    {
        _DestroyBuffer();
    }

    void DX11_ConstantBuffer::_DestroyBuffer()
    {
        if (m_ConstantBuffer != nullptr)
        {
            m_ConstantBuffer.Reset();
            AURORA_INFO(LogLayer::Graphics, "Constant Buffer: \"%s\" successfully destroyed.", GetObjectName().c_str());
        }
    }

    bool DX11_ConstantBuffer::Initialize(const std::string& constantBufferName, uint32_t bufferSize, DX11_Devices* devices)
    {
        AURORA_ASSERT(devices != nullptr);

        m_ObjectName = constantBufferName;
        m_Devices = devices;
        m_BufferSize = bufferSize;
        m_ObjectSizeGPU = static_cast<uint64_t>(bufferSize);
     
        // Destroy buffer if it existed previously.
        _DestroyBuffer();

        D3D11_BUFFER_DESC bufferDescription = {};
        bufferDescription.ByteWidth = static_cast<UINT>(bufferSize);
        bufferDescription.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
        bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDescription.MiscFlags = 0;
        bufferDescription.StructureByteStride = 0;

        const HRESULT result = m_Devices->m_Device->CreateBuffer(&bufferDescription, nullptr, (ID3D11Buffer**)m_ConstantBuffer.GetAddressOf());

        if (FAILED(result))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Constant Buffer.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Constant Buffer \"%s\" with size %f.", m_ObjectName.c_str(), static_cast<float>(m_ObjectSizeGPU));
        return true;
    }

    void* DX11_ConstantBuffer::Map()
    {
        AURORA_ASSERT(m_Devices != nullptr);
        AURORA_ASSERT(m_ConstantBuffer != nullptr);

        D3D11_MAPPED_SUBRESOURCE subresourceData;
        const HRESULT result = m_Devices->m_DeviceContextImmediate->Map(m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresourceData);
        if (FAILED(result))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to map Constant Buffer: \"%s\".", GetObjectName().c_str());
        }

        return subresourceData.pData;
    }

    bool DX11_ConstantBuffer::Unmap()
    {
        AURORA_ASSERT(m_Devices != nullptr);
        AURORA_ASSERT(m_ConstantBuffer != nullptr);

        m_Devices->m_DeviceContextImmediate->Unmap(m_ConstantBuffer.Get(), 0);

        return true;
    }
}
