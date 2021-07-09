#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../Graphics/RHI_Utilities.h"
#include "ShaderUtilities.h"

using namespace DirectX::PackedVector;

namespace Aurora
{
    enum Mesh_Flags
    {
        Mesh_Empty = 0,
        Mesh_Renderable = 1 << 0,
        Mesh_DoubleSided = 1 << 1
    };

    enum Compute_Normals_Type
    {
        Compute_Normals_Type_Hard,           // Hard face normals, can result in additional vertices generated.
        Compute_Normals_Type_Smooth,         // Smooth per vertex normals, this can remove/simply geometry, but is slow.
        Compute_Normals_Type_Smooth_Fast     // Average noramals, vertex count will be unchanged - fast.
    };

    // Position, Normal, Wind
    struct Vertex_Position
    {
        XMFLOAT3 m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        uint32_t m_Normal_Wind;

        XMVECTOR LoadPosition() const { return XMLoadFloat3(&m_Position); }
        XMVECTOR LoadNormal() const { XMFLOAT3 normal = GetNormal(); return XMLoadFloat3(&normal); }

        void PopulateFromParameters(const XMFLOAT3& position, const XMFLOAT3& normal, uint8_t wind)
        {
            m_Position.x = position.x;
            m_Position.y = position.y;
            m_Position.z = position.z;

            PopulateFromParameters(normal, wind);
        }

        void PopulateFromParameters(const XMFLOAT3& normal, uint8_t wind)
        {
            m_Normal_Wind = 0;

            // We are shifting bits here accordingly and finally casting the result to a uint32_t, allowing all the bits to line up accordingly. Finally, we assign it.
            m_Normal_Wind |= (uint32_t)((normal.x * 0.5f + 0.5f) * 255.0f) << 0;
            m_Normal_Wind |= (uint32_t)((normal.y * 0.5f + 0.5f) * 255.0f) << 8;
            m_Normal_Wind |= (uint32_t)((normal.z * 0.5f + 0.5f) * 255.0f) << 16;
            m_Normal_Wind |= (uint32_t)wind << 24;
        }

        void PopulateFromParameters(const XMFLOAT3& normal)
        {
            m_Normal_Wind = m_Normal_Wind & 0xFF000000; // Reset only the normals.

            m_Normal_Wind |= (uint32_t)((normal.x * 0.5f + 0.5f) * 255.0f) << 0;
            m_Normal_Wind |= (uint32_t)((normal.y * 0.5f + 0.5f) * 255.0f) << 8;
            m_Normal_Wind |= (uint32_t)((normal.z * 0.5f + 0.5f) * 255.0f) << 16;
        }

        XMFLOAT3 GetNormal() const
        {
            XMFLOAT3 normal(0, 0, 0);

            normal.x = (float)((m_Normal_Wind >> 0) & 0x000000FF) / 255.0f * 2.0f - 1.0f;
            normal.y = (float)((m_Normal_Wind >> 8) & 0x000000FF) / 255.0f * 2.0f - 1.0f;
            normal.z = (float)((m_Normal_Wind >> 16) & 0x000000FF) / 255.0f * 2.0f - 1.0f;

            return normal;
        }

        uint8_t GetWind() const
        {
            return (m_Normal_Wind >> 24) & 0x000000FF;
        }

        static const Format m_Format = Format::FORMAT_R32G32B32A32_FLOAT;
    };

    struct Vertex_TexCoord
    {
        XMHALF2 m_TexCoord = XMHALF2(0.0f, 0.0f);  // 16 bit floats.

        void PopulateFromParameters(const XMFLOAT2& texCoords)
        {
            m_TexCoord = XMHALF2(texCoords.x, texCoords.y);
        }

        static const Format m_Format = Format::FORMAT_R16G16_FLOAT;
    };

    struct Vertex_Color
    {
        uint32_t m_Color = 0;

        static const Format m_Format = Format::FORMAT_R8G8B8A8_UNORM;
    };

    struct Vertex_Tangent
    {
        uint32_t m_Tangent = 0;

        void PopulateFromParameters(const XMFLOAT4& tangent)
        {
            XMVECTOR Tangent = XMLoadFloat4(&tangent);
            Tangent = XMVector3Normalize(Tangent);

            XMFLOAT4 _tangent;
            XMStoreFloat4(&_tangent, Tangent);
           
            _tangent.w = tangent.w;
            m_Tangent = 0;
            m_Tangent |= (uint)((_tangent.x * 0.5f + 0.5f) * 255.0f) << 0;
            m_Tangent |= (uint)((_tangent.y * 0.5f + 0.5f) * 255.0f) << 8;
            m_Tangent |= (uint)((_tangent.z * 0.5f + 0.5f) * 255.0f) << 16;
            m_Tangent |= (uint)((_tangent.w * 0.5f + 0.5f) * 255.0f) << 24;
        }

        static const Format m_Format = Format::FORMAT_R8G8B8A8_UNORM;
    };
}