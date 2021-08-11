#pragma once
#include "../Resource/AuroraObject.h"
#include "../RHI_Implementation.h"

namespace Aurora
{
    class DX11_ConstantBuffer : public AuroraObject
    {
    public:
        DX11_ConstantBuffer() = default;
        ~DX11_ConstantBuffer();

        bool Initialize(const std::string& constantBufferName, uint32_t bufferSize, DX11_Devices* devices);
        uint32_t GetBufferSize() const { return m_BufferSize; }
        ComPtr<ID3D11Resource> GetConstantBuffer() const { return m_ConstantBuffer; }

        void* Map();
        bool Unmap();

    private:
        void _DestroyBuffer();

    private:
        uint32_t m_BufferSize = 0;
        DX11_Devices* m_Devices = nullptr;
        ComPtr<ID3D11Resource> m_ConstantBuffer = nullptr;
    };
}