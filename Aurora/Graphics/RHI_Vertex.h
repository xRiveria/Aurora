#pragma once
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
        VertexType_PositionUVNormal
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