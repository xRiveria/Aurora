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
}