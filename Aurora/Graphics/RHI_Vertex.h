#pragma once
#include "RHI_Implementation.h"
#include <DirectXMath.h>

/* === RHI Vertex ===

    Contains all possible vertex formats for our shaders. We will be switching our DirectX for our own Math library in due time.
*/

using namespace DirectX;

namespace Aurora
{
    enum class RHI_Vertex_Type
    {
        VertexType_Unknown,
        VertexType_Position,
        VertexType_PositionUV,
        VertexType_PositionColor,
        VertexType_PositionUVNormal,
    };

    // To remake into fully RHI attribute.
    struct VertexAttribute
    {
        VertexAttribute(std::string semanticName, UINT semanticIndex, DXGI_FORMAT format, UINT inputSlot, UINT byteOffset, D3D11_INPUT_CLASSIFICATION inputClassification)
        {
            this->m_SemanticName = semanticName;
            this->m_SemanticIndex = semanticIndex;
            this->m_Format = format;
            this->m_InputSlot = inputSlot;
            this->m_ByteOffset = byteOffset;
            this->m_InputClassification = inputClassification;
        }

        std::string m_SemanticName;
        UINT m_SemanticIndex; // Allows us to reuse an existing semantic name.
        DXGI_FORMAT m_Format;
        UINT m_InputSlot; // Tells us which bound vertex buffer provides the data for the attribute.
        UINT m_ByteOffset;
        D3D11_INPUT_CLASSIFICATION m_InputClassification;
    };

    struct RHI_Vertex_Position
    {
        RHI_Vertex_Position(const XMFLOAT3& position)
        {
            this->m_Position.x = position.x;
            this->m_Position.y = position.y;
            this->m_Position.z = position.z;
        }

        XMFLOAT3 m_Position = { 0, 0, 0 };
    };

    struct RHI_Vertex_Position_UV
    {
        RHI_Vertex_Position_UV(const XMFLOAT3& position, const XMFLOAT2& uv)
        {
            this->m_Position.x = position.x;
            this->m_Position.y = position.y;
            this->m_Position.z = position.z;

            this->m_UV.x = uv.x;
            this->m_UV.y = uv.y;
        }

        XMFLOAT3 m_Position = { 0, 0, 0 };
        XMFLOAT2 m_UV = { 0, 0 };
    };

    struct RHI_Vertex_Position_Color
    {
        RHI_Vertex_Position_Color() = default;

        RHI_Vertex_Position_Color(const XMFLOAT3& position, const XMFLOAT4& color)
        {
            this->m_Position.x = position.x;
            this->m_Position.y = position.y;
            this->m_Position.z = position.z;
            
            this->m_Color.x = color.x;
            this->m_Color.y = color.y;
            this->m_Color.z = color.z;
            this->m_Color.w = color.w;
        }

        XMFLOAT3 m_Position = { 0.0f, 0.0f, 0.0f };
        XMFLOAT4 m_Color = { 0.0f, 0.0f, 0.0f, 0.0f };
    };

    struct RHI_Vertex_Position_UV_Normal
    {
        RHI_Vertex_Position_UV_Normal() {}
        RHI_Vertex_Position_UV_Normal(const XMFLOAT3& position, const XMFLOAT2& uv, const XMFLOAT3& normal)
        {
            this->m_Position.x = position.x;
            this->m_Position.y = position.y;
            this->m_Position.z = position.z;

            this->m_UV.x = uv.x;
            this->m_UV.y = uv.y;

            this->m_Normal.x = normal.x;
            this->m_Normal.y = normal.y;
            this->m_Normal.z = normal.z;
        }

        XMFLOAT3 m_Position = { 0, 0, 0 };
        XMFLOAT2 m_UV = { 0, 0 };
        XMFLOAT3 m_Normal = { 0, 0, 0 };
    };
}