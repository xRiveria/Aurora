#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Material.h"
#include <DirectXPackedVector.h>

using namespace DirectX::PackedVector;

namespace Aurora
{
    struct MeshComponent
    {
        std::vector<uint32_t> m_Indices; ///
        IndexBuffer_Format GetIndexFormat() const { return m_VertexPositions.size() > 65535 ? IndexBuffer_Format::Format_32Bit : IndexBuffer_Format::Format_16Bit; } ///

        std::vector<XMFLOAT3> m_VertexPositions;
        std::vector<XMFLOAT3> m_VertexNormals;
        std::vector<XMFLOAT2> m_UVSet_0;

        RHI_GPU_Buffer m_IndexBuffer;
        RHI_GPU_Buffer m_VertexBuffer_Position;

        std::shared_ptr<AuroraResource> m_BaseTexture = nullptr;

        EngineContext* m_EngineContext;
    };
   
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

    class World : public ISubsystem
    {
    public:
        World(EngineContext* engineContext);
        ~World();

        // Entity
        void DrawScene();

        /// Make this return an entity once our ECS is implemented.
        void LoadModel(const std::string& filePath);
        void ImportModel_OBJ(const std::string& filePath);


        void CreateRenderData();
        MeshComponent m_MeshComponent;
    };
}