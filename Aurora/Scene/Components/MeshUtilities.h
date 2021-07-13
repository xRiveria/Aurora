#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../Graphics/RHI_Utilities.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

namespace Aurora
{
    // Normals. 
    struct Vertex_Position
    {
        XMFLOAT3 m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMFLOAT2 m_TexCoord = XMFLOAT2(0.0f, 0.0f);
        XMFLOAT3 m_Normals = XMFLOAT3(0.0f, 0.0f, 0.0f);

        static const Format m_Format = Format::FORMAT_R32G32B32A32_FLOAT;  // XYZW

        void Populate(const XMFLOAT3& position, const XMFLOAT2& texCoords, const XMFLOAT3& normals)
        {
            m_Position.x = position.x;
            m_Position.y = position.y;
            m_Position.z = position.z;

            m_TexCoord.x = texCoords.x;
            m_TexCoord.y = texCoords.y;

            m_Normals.x = normals.x;
            m_Normals.y = normals.y;
            m_Normals.z = normals.z;
        }

        XMVECTOR LoadPosition() const
        {
            return XMLoadFloat3(&m_Position);
        }
    };

    struct Vertex_TexCoords
    {
        XMHALF2 m_TexCoords = XMHALF2(0.0f, 0.0f);

        void Populate(const XMFLOAT2& texCoords)
        {
            m_TexCoords = XMHALF2(texCoords.x, texCoords.y);
        }

        static const Format m_Format = Format::FORMAT_R16G16_FLOAT;
    };
}