#include "Aurora.h"
#include "DX11_VertexBuffer.h"
#include "DX11_IndexBuffer.h"
#include "DX11_ConstantBuffer.h"
#include "DX11_Context.h"

namespace Aurora
{
    DX11_Context::DX11_Context(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext)
    {
        m_Devices = std::make_shared<DX11_Devices>();

        m_Devices->m_Device = device.Get();
        m_Devices->m_DeviceContextImmediate = deviceContext.Get();
    }

    std::shared_ptr<DX11_IndexBuffer> DX11_Context::CreateIndexBuffer(std::vector<uint32_t>& indices)
    {
        std::shared_ptr<DX11_IndexBuffer> indexBuffer = std::make_shared<DX11_IndexBuffer>();
        indexBuffer->Initialize(indices, m_Devices.get());
        return indexBuffer;
    }
}