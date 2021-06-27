#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    struct MeshComponent
    {
        std::vector<XMFLOAT3> m_VertexPositions;
        std::vector<XMFLOAT2> m_UVSet_0;
        std::vector<uint32_t> m_Indices;

        RHI_GPU_Buffer m_IndexBuffer;
        RHI_GPU_Buffer m_VertexBuffer_Position;

        std::shared_ptr<AuroraResource> m_BaseTexture = nullptr;
        IndexBuffer_Format GetIndexFormat() const { return m_VertexPositions.size() > 65535 ? IndexBuffer_Format::Format_32Bit : IndexBuffer_Format::Format_16Bit; }

        EngineContext* m_EngineContext;
    };
   
    // Normals. 
    struct Vertex_Position
    {
        XMFLOAT3 m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMFLOAT2 m_TexCoord = XMFLOAT2(0.0f, 0.0f);
        static const Format m_Format = Format::FORMAT_R32G32B32A32_FLOAT;  // XYZW

        void Populate(const XMFLOAT3& position, const XMFLOAT2& texCoords)
        {
            m_Position.x = position.x;
            m_Position.y = position.y;
            m_Position.z = position.z;
            m_TexCoord.x = texCoords.x;
            m_TexCoord.y = texCoords.y;
        }

        XMVECTOR LoadPosition() const
        {
            return XMLoadFloat3(&m_Position);
        }
    };

    class World : public ISubsystem
    {
    public:
        World(EngineContext* engineContext);
        ~World();

        // Entity

        /// Make this return an entity once our ECS is implemented.
        void LoadModel(const std::string& filePath);
        void ImportModel_OBJ(const std::string& filePath);


        void CreateRenderData();
        MeshComponent m_MeshComponent;
    };
}