#include "Aurora.h"
#include "DX11_InputLayout.h"

namespace Aurora
{
    DX11_InputLayout::~DX11_InputLayout()
    {
        if (m_InputLayout != nullptr)
        {
            AURORA_INFO(LogLayer::Graphics, "Input Layout successfully released.");
            m_InputLayout.Reset();
        }
    }

    bool DX11_InputLayout::Initialize(RHI_Vertex_Type vertexType, std::vector<uint8_t>& vertexShaderBlob, DX11_Devices* devices)
    {
        AURORA_ASSERT(devices != nullptr);
        AURORA_ASSERT(vertexShaderBlob.data() != nullptr);

        m_Devices = devices;
        m_VertexType = vertexType;

        UINT inputSlotBinding = 0;
        switch (vertexType)
        {
        case RHI_Vertex_Type::VertexType_Unknown:
            AURORA_ERROR(LogLayer::Graphics, "Unknown Vertex Type is being used to create Input Layout.");
            return false;

        case RHI_Vertex_Type::VertexType_Position:
            m_VertexAttributes =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlotBinding, offsetof(RHI_Vertex_Position, m_Position), D3D11_INPUT_PER_VERTEX_DATA  }
            };
            return _Initialize(vertexShaderBlob);

        case RHI_Vertex_Type::VertexType_PositionUV:
            m_VertexAttributes =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlotBinding, offsetof(RHI_Vertex_Position_UV, m_Position), D3D11_INPUT_PER_VERTEX_DATA },
                { "UV",       0, DXGI_FORMAT_R32G32_FLOAT,    inputSlotBinding, offsetof(RHI_Vertex_Position_UV, m_UV), D3D11_INPUT_PER_VERTEX_DATA }
            };
            return _Initialize(vertexShaderBlob);

        case RHI_Vertex_Type::VertexType_PositionColor:
            m_VertexAttributes =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlotBinding, offsetof(RHI_Vertex_Position_Color, m_Position), D3D11_INPUT_PER_VERTEX_DATA },
                { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, inputSlotBinding, offsetof(RHI_Vertex_Position_Color, m_Color), D3D11_INPUT_PER_VERTEX_DATA }
            };
            return _Initialize(vertexShaderBlob);

        case RHI_Vertex_Type::VertexType_PositionUVNormal:
            m_VertexAttributes =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlotBinding, offsetof(RHI_Vertex_Position_UV_Normal, m_Position), D3D11_INPUT_PER_VERTEX_DATA },
                { "UV",       0, DXGI_FORMAT_R32G32_FLOAT,    inputSlotBinding, offsetof(RHI_Vertex_Position_UV_Normal, m_UV),       D3D11_INPUT_PER_VERTEX_DATA },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlotBinding, offsetof(RHI_Vertex_Position_UV_Normal, m_Normal),   D3D11_INPUT_PER_VERTEX_DATA }
            };
            return _Initialize(vertexShaderBlob);
        }        
    }

    bool DX11_InputLayout::_Initialize(std::vector<uint8_t>& vertexShaderBlob)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> attributeDescriptions;

        for (const VertexAttribute& attribute : m_VertexAttributes)
        {
            attributeDescriptions.emplace_back(D3D11_INPUT_ELEMENT_DESC {
                attribute.m_SemanticName.c_str(),
                attribute.m_SemanticIndex,
                attribute.m_Format,
                attribute.m_InputSlot,
                attribute.m_ByteOffset,
                attribute.m_InputClassification,
                0
            });
        }

        const HRESULT result = m_Devices->m_Device->CreateInputLayout(attributeDescriptions.data(), static_cast<UINT>(attributeDescriptions.size()), vertexShaderBlob.data(), vertexShaderBlob.size(), &m_InputLayout);

        if (FAILED(result))
        {
            AURORA_ERROR(LogLayer::Graphics, "Failed to create Input Layout.");
            return false;
        }

        AURORA_INFO(LogLayer::Graphics, "Successfully created Input Layout with %f Elements.", static_cast<float>(m_VertexAttributes.size()));
        return true;
    }
}