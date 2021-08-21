#include "Aurora.h"
#include "Model.h"
#include "Mesh.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Components/Renderable.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    Model::Model(EngineContext* engineContext) : AuroraResource(engineContext, ResourceType::ResourceType_Model)
    {
        m_ResourceCache = engineContext->GetSubsystem<ResourceCache>();
        m_Mesh = std::make_unique<Mesh>(); // Unique pointers can be converted to shared pointers, but not vice versa. This means that we can change our mind from using unique pointers whenever.
    }

    void Model::Clear()
    {
        m_RootEntity.reset();
        m_VertexBuffer.reset();
        m_IndexBuffer.reset();
        m_Mesh->Clear();
        m_IsAnimated = false;
    }

    bool Model::LoadFromFile(const std::string& filePath)
    {
        const Stopwatch stopwatch("Model Loading", false);

        if (filePath.empty() || FileSystem::IsDirectory(filePath))
        {
            AURORA_WARNING(LogLayer::Engine, "Invalid Model File Path: %s", filePath.c_str());
            return false;
        }

        // Load engine format.
        if (FileSystem::GetExtensionFromFilePath(filePath) == EXTENSION_MODEL)
        {
            // Deserialize
            std::unique_ptr<BinarySerializer> binarySerializer = std::make_unique<BinarySerializer>(filePath, SerializerFlag::SerializerMode_Read);
            if (!binarySerializer->IsStreamOpen())
            {
                return false;
            }

            SetResourceFilePath(binarySerializer->ReadAs<std::string>());
            binarySerializer->Read(&m_Mesh->GetIndices());
            binarySerializer->Read(&m_Mesh->GetVertexPositions());
            binarySerializer->Read(&m_Mesh->GetVertexNormals());
            binarySerializer->Read(&m_Mesh->GetVertexUVs());

            CreateBuffers();
        }
        else
        {
            /// Load foreign format.
            SetResourceFilePath(filePath);

            if (m_ResourceCache->GetModelImporter()->LoadModel(filePath, this))
            {
                /// Set the normalized scale to the root entity's transform.

            }
            else
            {
                return false;
            }
        }

        return true;
    }

    // Binary serialization as YAML will take too slow to save/load the amount of vertices/indices.
    bool Model::SaveToFile(const std::string& filePath)
    {
        std::unique_ptr<BinarySerializer> binarySerializer = std::make_unique<BinarySerializer>(filePath, SerializerFlag::SerializerMode_Write);
        if (!binarySerializer->IsStreamOpen())
        {
            return false;
        }

        binarySerializer->Write(GetResourceFilePath());
        binarySerializer->Write(m_Mesh->GetIndices());
        binarySerializer->Write(m_Mesh->GetVertexPositions());
        binarySerializer->Write(m_Mesh->GetVertexNormals());
        binarySerializer->Write(m_Mesh->GetVertexUVs());

        binarySerializer->CloseStream();

        return true;
    }

    void Model::AppendGeometry(const std::vector<uint32_t>& indices, const std::vector<XMFLOAT3>& vertexPositions, const std::vector<XMFLOAT3>& vertexNormals, const std::vector<XMFLOAT2>& vertexUVs)
    {
        // Append vertices and indices to the main mesh.
        m_Mesh->SetVertexPositions(vertexPositions);
        m_Mesh->SetVertexNormals(vertexNormals);
        m_Mesh->SetVertexUVs(vertexUVs);
        m_Mesh->SetIndices(indices);
    }

    void Model::AddMaterial(std::shared_ptr<Material>& material, const std::shared_ptr<Entity>& entity)
    {
        AURORA_ASSERT(material != nullptr);
        AURORA_ASSERT(entity != nullptr);

        // We will set the material's file path at this model's directory.
        const std::string assetPath = FileSystem::GetDirectoryFromFilePath(GetResourceFilePathNative()) + material->GetResourceName() + EXTENSION_MATERIAL;
        material->SetResourceFilePath(assetPath);

        // Create a Renderable component (or retrieve it if it already exists) and pass the material to it.
        entity->AddComponent<Renderable>()->SetMaterial(material);
    }

    bool Model::CreateBuffers()
    {
        AURORA_ASSERT(m_Mesh->GetVerticesCount() != 0);
        AURORA_ASSERT(m_Mesh->GetIndicesCount() != 0);

        std::shared_ptr<DX11_Context>& rendererContext = m_EngineContext->GetSubsystem<Renderer>()->m_DeviceContext;
        m_IndexBuffer = rendererContext->CreateIndexBuffer(m_Mesh->GetIndices());

        // Grab vertex positions and store them in our own data structure.
        std::vector<RHI_Vertex_Position_UV_Normal> m_VertexPositionUVNormal(m_Mesh->GetVerticesCount());

        bool isVertexNormalsEmpty = false;
        bool isTexCoordsEmpty = false;

        if (m_Mesh->GetVertexNormals().empty())
        {
            isVertexNormalsEmpty = true;
        }
        if (m_Mesh->GetVertexUVs().empty())
        {
            isTexCoordsEmpty = true;
        }

        for (size_t i = 0; i < m_VertexPositionUVNormal.size(); ++i)
        {
            const XMFLOAT3& position = m_Mesh->GetVertexPositions()[i];
            XMFLOAT3 texNormals = { 0, 0, 0 };
            XMFLOAT2 texCoords = { 0, 0 };

            if (!isVertexNormalsEmpty)
            {
                texNormals = m_Mesh->GetVertexNormals()[i];
            }

            if (!isTexCoordsEmpty)
            {
                texCoords = m_Mesh->GetVertexUVs()[i];
            }

            m_VertexPositionUVNormal[i].m_Position = position;
            m_VertexPositionUVNormal[i].m_UV = texCoords;
            m_VertexPositionUVNormal[i].m_Normal = texNormals;
        }

        m_VertexBuffer = rendererContext->CreateVertexBuffer(RHI_Vertex_Type::VertexType_PositionUVNormal, m_VertexPositionUVNormal);

        return true;
    }
}
