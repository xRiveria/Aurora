#pragma once
#include "../Resource/AuroraObject.h"
#include "../RHI_Implementation.h"
#include "../RHI_Vertex.h"

namespace Aurora
{
    class DX11_InputLayout : public AuroraObject
    {
    public:
        DX11_InputLayout() = default;
        ~DX11_InputLayout();

        bool Initialize(RHI_Vertex_Type vertexType, std::vector<uint8_t>& vertexShaderBlob, DX11_Devices* devices);

        const auto& GetAttributeDescriptions() const { return m_VertexAttributes; }
        RHI_Vertex_Type GetVertexType() const { return m_VertexType; }
        ComPtr<ID3D11InputLayout> GetInputLayout() const { return m_InputLayout; }

    private:
        bool _Initialize(std::vector<uint8_t>& vertexShaderBlob);

    private:
        RHI_Vertex_Type m_VertexType;

        std::vector<VertexAttribute> m_VertexAttributes;
        ComPtr<ID3D11InputLayout> m_InputLayout;
        DX11_Devices* m_Devices = nullptr;
    };
}