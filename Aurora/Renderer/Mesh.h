#pragma once
#include "../Resource/AuroraResource.h"
#include "../Renderer/Material.h"

/* === Mesh ===
    - A mesh, simplified to its core, is a collection of positions, normals and other arbitrary data as far as rendering is concerned.
    - Our mesh class is extremely simplified, containing only pure data loaded from an importer and has no engine code that ties it to other systems.

    - All forms of mesh manipulation will go through its Model class.
*/

namespace Aurora
{
    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() { Clear(); }

        void Clear();

        // Indices
        void AppendIndices(const std::vector<uint32_t>& indices, uint32_t* indexOffset);
        uint32_t GetIndicesCount() const { return static_cast<uint32_t>(m_Indices.size()); }
        std::vector<uint32_t>& GetIndices() { return m_Indices; }
        void SetIndices(const std::vector<uint32_t>& indices) { m_Indices = indices; }

        // Vertices
        void AppendVertices(const std::vector<XMFLOAT3>& vertexPositions, const std::vector<XMFLOAT3>& vertexNormals, const std::vector<XMFLOAT2>& vertexUVs, uint32_t* vertexOffset);
        uint32_t GetVerticesCount() const { return static_cast<uint32_t>(m_VertexPositions.size()); }

        std::vector<XMFLOAT3>& GetVertexPositions() { return m_VertexPositions; }
        void SetVertexPositions(const std::vector<XMFLOAT3>& vertexPositions) { m_VertexPositions = vertexPositions; }
        std::vector<XMFLOAT3>& GetVertexNormals() { return m_VertexNormals; }
        void SetVertexNormals(const std::vector<XMFLOAT3>& vertexNormals) { m_VertexNormals = vertexNormals; }
        std::vector<XMFLOAT2>& GetVertexUVs() { return m_UVSet_0; }
        void SetVertexUVs(const std::vector<XMFLOAT2>& vertexUVs) { m_UVSet_0 = vertexUVs; }

        IndexBuffer_Format GetIndexFormat() const { return m_VertexPositions.size() > 65535 ? IndexBuffer_Format::Format_32Bit : IndexBuffer_Format::Format_16Bit; } // Try to save memory/bandwidth whenever we can.

    public:
        // Information retrieved from the mesh.
        std::vector<uint32_t> m_Indices;

        std::vector<XMFLOAT3> m_VertexPositions;
        std::vector<XMFLOAT3> m_VertexNormals;
        std::vector<XMFLOAT2> m_UVSet_0;
    };
}