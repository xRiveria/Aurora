#pragma once
#include "../Graphics/RHI_Implementation.h"
#include "MeshUtilities.h"
#include "../Entity.h"
#include "ShaderInternals.h"

/*
    A mesh component will consist of vertex, bones, normals information amongst others. As long as a mesh component exists on an entity, there will be some form of rendering as long as there 
    is vertex data to utilized. A mesh can have multiple parts, called MeshSubsets. Each MeshSubset has a material and it is using a range of triangles of the mesh. This can also have GPU resident data for rendering.

    A material component on the other hand, maps textures.
*/
namespace Aurora
{
    class Mesh : public IComponent
    {
    public:
        Mesh(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);

        void SetRenderable(bool value)  { if (value) { m_Flags |= Mesh_Flags::Mesh_Renderable;  } else { m_Flags &= ~Mesh_Flags::Mesh_Renderable;  } }
        void SetDoubleSided(bool value) { if (value) { m_Flags |= Mesh_Flags::Mesh_DoubleSided; } else { m_Flags &= ~Mesh_Flags::Mesh_DoubleSided; } }

        bool IsRenderable() const { return m_Flags & Mesh_Flags::Mesh_Renderable; }
        bool IsDoubleSided() const { return m_Flags & Mesh_Flags::Mesh_DoubleSided; }

        IndexBuffer_Format GetIndexFormat() const { return m_Vertex_Positions.size() > 65535 ? IndexBuffer_Format::Format_32Bit : IndexBuffer_Format::Format_16Bit; }  // Bandwidth and memory can be saved by optimizing the type whenever possible.
        size_t GetIndexStride() const { return GetIndexFormat() == IndexBuffer_Format::Format_32Bit ? sizeof(uint32_t) : sizeof(uint16_t); }

        // Recreates GPU resources for the index/vertex buffers.
        void CreateRenderData();
        void WriteToShaderMesh(ShaderMesh* destination) const;

        void ComputeNormals(Compute_Normals_Type computeType);
        void FlipCulling();
        void FlipNormals();

    public:
        struct MeshSubset
        {
            Entity* m_Material_Entity = nullptr;
            uint32_t m_Index_Offset = 0;
            uint32_t m_Index_Count = 0;

            uint32_t m_Material_Index = 0;
        };

        std::vector<MeshSubset> m_Mesh_Subsets;

        // Mesh Information

        std::vector<XMFLOAT3> m_Vertex_Positions;
        std::vector<XMFLOAT3> m_Vertex_Normals;
        std::vector<XMFLOAT4> m_Vertex_Tangents;
        std::vector<XMFLOAT2> m_Vertex_UVSet_0;
        std::vector<XMFLOAT2> m_Vertex_UVSet_1;
        // std::vector<XMUINT4>  m_Vertex_BoneIndices;
        // std::vector<XMFLOAT4> m_Vertex_BoneWeights;
        // std::vector<XMFLOAT2> m_Vertex_Atlas;
        std::vector<uint32_t> m_Vertex_Colors;
        std::vector<uint8_t>  m_Vertex_WindWeights;
        std::vector<uint32_t> m_Indices;

        // Data Buffers
        RHI_GPU_Buffer m_Index_Buffer;
        RHI_GPU_Buffer m_Vertex_Buffer_Position;  // Consists of Positions, Normals and Wind information. We will set the wind information to 0 for now.
        RHI_GPU_Buffer m_Vertex_Buffer_Tangent;
        RHI_GPU_Buffer m_Vertex_Buffer_UV0;
        RHI_GPU_Buffer m_Vertex_Buffer_UV1;
        RHI_GPU_Buffer m_Vertex_Buffer_Color;

        std::vector<MeshSubset> m_Subsets;
        RHI_GPU_Buffer m_Vertex_Buffer_Subsets;
        std::vector<uint8_t> m_Vertex_Subsets;
        /// Bones.

    private:
        uint32_t m_Flags = Mesh_Flags::Mesh_Renderable;

        EngineContext* m_EngineContext;
    };
}