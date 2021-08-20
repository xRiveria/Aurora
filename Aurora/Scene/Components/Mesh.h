#pragma once
#include "IComponent.h"
#include "../Resource/AuroraResource.h"
#include "../Renderer/Material.h"

namespace Aurora
{
    class Mesh : public IComponent
    {
    public:
        Mesh(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);

        void Serialize(SerializationStream& outputStream) override;
        void Deserialize(SerializationNode& inputNode) override;

        void CreateRenderData();

        // Material - Sets a material from memory (adds it to the resource cache by default).
        void AddMaterial(const std::shared_ptr<Material>& material);

        // Loads a material and then sets it.
        std::shared_ptr<Material> SetMaterial(const std::shared_ptr<Material>& material);
        std::shared_ptr<Material> SetMaterial(const std::string& filePath);

    public:
        IndexBuffer_Format GetIndexFormat() const { return m_VertexPositions.size() > 65535 ? IndexBuffer_Format::Format_32Bit : IndexBuffer_Format::Format_16Bit; } // Try to save memory/bandwidth whenever we can.

    public:
        // Information retrieved from the mesh.
        std::vector<uint32_t> m_Indices;

        std::vector<XMFLOAT3> m_VertexPositions;
        std::vector<XMFLOAT3> m_VertexNormals;
        std::vector<XMFLOAT2> m_UVSet_0;

        // Abstract away this information and store it in our newly created DX11_MeshData struct.
        std::shared_ptr<DX11_MeshData> m_MeshData = nullptr;

        Material* m_Material;

        // RHI_GPU_Buffer m_IndexBuffer;
        // RHI_GPU_Buffer m_VertexBuffer_Position;
    };
}