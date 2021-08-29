#include "Aurora.h"
#include "Mesh.h"

namespace Aurora
{
    void Mesh::Clear()
    {
        m_VertexPositions.clear();
        m_VertexPositions.shrink_to_fit();

        m_VertexNormals.clear();
        m_VertexNormals.shrink_to_fit();

        m_UVSet_0.clear();
        m_UVSet_0.shrink_to_fit();

        m_Indices.clear();
        m_Indices.shrink_to_fit();
    }

    void Mesh::AppendIndices(const std::vector<uint32_t>& indices, uint32_t* indexOffset)
    {
        if (indexOffset)
        {
            *indexOffset = static_cast<uint32_t>(m_Indices.size());
        }

        m_Indices.insert(m_Indices.end(), indices.begin(), indices.end());
    }

    void Mesh::AppendVertices(const std::vector<XMFLOAT3>& vertexPositions, const std::vector<XMFLOAT3>& vertexNormals, const std::vector<XMFLOAT2>& vertexUVs, uint32_t* vertexOffset)
    {
        if (vertexOffset)
        {
            *vertexOffset = static_cast<uint32_t>(m_VertexPositions.size());
        }

        m_VertexPositions.insert(m_VertexPositions.end(), vertexPositions.begin(), vertexPositions.end());
        m_VertexNormals.insert(m_VertexNormals.end(), vertexNormals.begin(), vertexNormals.end());
        m_UVSet_0.insert(m_UVSet_0.end(), vertexUVs.begin(), vertexUVs.end());
    }
}