#pragma once
#include "AuroraObject.h"
#include "../RHI_Implementation.h"

namespace Aurora
{
    class DX11_IndexBuffer : public AuroraObject
    {
    public:
        DX11_IndexBuffer() = default;
        ~DX11_IndexBuffer();

        bool Initialize(std::vector<uint32_t>& indices, DX11_Devices* devices);
        void* Map();
        bool Unmap();

        bool Is16Bit() const { return m_Stride == sizeof(uint16_t); }
        bool Is64Bit() const { return m_Stride == sizeof(uint32_t); }
        uint32_t GetStride() const { return m_Stride; }
        uint32_t GetOffset() const { return m_Offset; }
        uint32_t GetIndexCount() const { return m_IndexCount; }
        ID3D11Buffer* GetIndexBuffer() const { return (ID3D11Buffer*)m_IndexBuffer.Get(); }

    private:
        void _DestroyBuffer();

    private:
        uint32_t m_Stride = 0;
        uint32_t m_Offset = 0;
        uint32_t m_IndexCount = 0;

        ComPtr<ID3D11Resource> m_IndexBuffer = nullptr;
        DX11_Devices* m_Devices = nullptr;
    };
}
