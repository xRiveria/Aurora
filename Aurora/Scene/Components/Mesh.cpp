#include "Aurora.h"
#include "Mesh.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    Mesh::Mesh(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    void Mesh::Serialize(SerializationStream& outputStream)
    {
        outputStream << YAML::Key << "MeshComponent";
        outputStream << YAML::BeginMap;

        // ==========================================================

        for (int i = 0; i < m_VertexPositions.size(); i++)
        {
            outputStream << YAML::Key << "VertexPosition_" + std::to_string(i) << YAML::Value << m_VertexPositions[i];
        }
        outputStream << YAML::Key << "VertexPositionsSize" << YAML::Value << m_VertexPositions.size();

        // ==========================================================

        for (int j = 0; j < m_VertexNormals.size(); j++)
        {
            outputStream << YAML::Key << "VertexNormal_" + std::to_string(j) << YAML::Value << m_VertexNormals[j];
        }
        outputStream << YAML::Key << "VertexNormalsSize" << YAML::Value << m_VertexNormals.size();

        // ==========================================================

        for (int k = 0; k < m_UVSet_0.size(); k++)
        {
            outputStream << YAML::Key << "UVSet0_" + std::to_string(k) << YAML::Value << m_UVSet_0[k];
        }
        outputStream << YAML::Key << "VertexUVSetsSize" << YAML::Value << m_UVSet_0.size();

        // ==========================================================

        for (int w = 0; w < m_Indices.size(); w++)
        {
            outputStream << YAML::Key << "Indice_" + std::to_string(w) << YAML::Value << m_Indices[w];
        }
        outputStream << YAML::Key << "IndicesSize" << YAML::Value << m_Indices.size();

        outputStream << YAML::EndMap;
    }

    void Mesh::Deserialize(SerializationNode& inputNode)
    {
        uint32_t vertexPositionsSize = inputNode["VertexPositionsSize"].as<uint32_t>();
        m_VertexPositions.reserve(vertexPositionsSize);

        for (uint32_t i = 0; i < vertexPositionsSize; i++)
        {
            XMFLOAT3 position = inputNode["VertexPosition_" + std::to_string(i)].as<XMFLOAT3>();
            m_VertexPositions.push_back(position);
        }

        // ==========================================================

        uint32_t vertexNormalsSize = inputNode["VertexNormalsSize"].as<uint32_t>();
        m_VertexNormals.reserve(vertexNormalsSize);

        for (uint32_t i = 0; i < vertexNormalsSize; i++)
        {
            XMFLOAT3 normal = inputNode["VertexNormal_" + std::to_string(i)].as<XMFLOAT3>();
            m_VertexNormals.push_back(normal);
        }

        // ==========================================================

        uint32_t uvSetsSize = inputNode["VertexUVSetsSize"].as<uint32_t>();
        m_UVSet_0.reserve(uvSetsSize);

        for (uint32_t i = 0; i < uvSetsSize; i++)
        {
            XMFLOAT2 uvSet = inputNode["UVSet0_" + std::to_string(i)].as<XMFLOAT2>();
            m_UVSet_0.push_back(uvSet);
        }

        // ==========================================================

        uint32_t indicesSize = inputNode["IndicesSize"].as<uint32_t>();
        m_Indices.reserve(indicesSize);

        for (uint32_t i = 0; i < indicesSize; i++)
        {
            uint32_t indice = inputNode["Indice_" + std::to_string(i)].as<uint32_t>();
            m_Indices.push_back(indice);
        }

        CreateRenderData();
    }

    void Mesh::CreateRenderData()
    {
        DX11_GraphicsDevice* graphicsDevice = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice.get();

        // Create Index Buffer GPU Data
        RHI_GPU_Buffer_Description indexBufferDescription;
        indexBufferDescription.m_Usage = Usage::Immutable;
        indexBufferDescription.m_CPUAccessFlags = 0;
        indexBufferDescription.m_BindFlags = Bind_Flag::Bind_Index_Buffer | Bind_Flag::Bind_Shader_Resource;
        indexBufferDescription.m_MiscFlags = 0;

        RHI_Subresource_Data indexInitializationData;

        // Create buffer for our index data - 16 bit.
        if (GetIndexFormat() == IndexBuffer_Format::Format_32Bit)
        {
            indexBufferDescription.m_StructureByteStride = sizeof(uint32_t);
            indexBufferDescription.m_Format = Format::FORMAT_R32_UINT;
            indexBufferDescription.m_ByteWidth = uint32_t(sizeof(uint32_t) * m_Indices.size());

            // Use indices directly since vector is in correct format.
            static_assert(std::is_same<decltype(m_Indices)::value_type, uint32_t>::value, "Indices not in Index_Format::32_Bit.");
            indexInitializationData.m_SystemMemory = m_Indices.data();

            graphicsDevice->CreateBuffer(&indexBufferDescription, &indexInitializationData, &m_IndexBuffer);
            /// Set name.
        }
        // 16 Bit
        else
        {
            indexBufferDescription.m_StructureByteStride = sizeof(uint16_t);
            indexBufferDescription.m_Format = Format::FORMAT_R16_UINT;
            indexBufferDescription.m_ByteWidth = uint32_t(sizeof(uint16_t) * m_Indices.size());

            std::vector<uint16_t> gpuIndexBuffer(m_Indices.size()); // 16 bit buffer.
            std::copy(m_Indices.begin(), m_Indices.end(), gpuIndexBuffer.begin());
            indexInitializationData.m_SystemMemory = gpuIndexBuffer.data();

            graphicsDevice->CreateBuffer(&indexBufferDescription, &indexInitializationData, &m_IndexBuffer);
            /// Set name.
        }

        // For AABB in the future.
        XMFLOAT3 minimumVector = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
        XMFLOAT3 maximumVector = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // Grab vertex positions and store them in our own data structure.
        std::vector<Vertex_Position> vertices(m_VertexPositions.size());


        bool isVertexNormalsEmpty = false;
        bool isTexCoordsEmpty = false;
        if (m_VertexNormals.empty())
        {
            isVertexNormalsEmpty = true;
        }
        if (m_UVSet_0.empty())
        {
            isTexCoordsEmpty = true;
        }

        for (size_t i = 0; i < vertices.size(); ++i)
        {
            /// Normals.
            /// Wind Weights.
            const XMFLOAT3& position = m_VertexPositions[i];
           
            XMFLOAT3 texNormals = { 0, 0, 0 };
            XMFLOAT2 texCoords = { 0, 0 };
          
            if (!isVertexNormalsEmpty)
            {
                texNormals = m_VertexNormals[i];
            }

            if (!isTexCoordsEmpty)
            {
                texCoords = m_UVSet_0[i];
            }

            vertices[i].Populate(position, texCoords, m_VertexNormals[i]);

            // minimumVector = Aurora::Math::Minimum(minimumVector, position);
            // maximumVector = Aurora::Math::Maximum(maximumVector, position);
        }

        // Create buffer for our vertex data (positions).
        RHI_GPU_Buffer_Description vertexBufferDescription;
        vertexBufferDescription.m_Usage = Usage::Default;
        vertexBufferDescription.m_CPUAccessFlags = 0;
        vertexBufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer | Bind_Flag::Bind_Shader_Resource;
        vertexBufferDescription.m_MiscFlags = Resource_Misc_Flag::Resource_Misc_Buffer_Allow_Raw_Views;
        /// Check Raytracing.
        vertexBufferDescription.m_ByteWidth = (uint32_t)(sizeof(Vertex_Position) * vertices.size());

        RHI_Subresource_Data vertexInitializationData;
        vertexInitializationData.m_SystemMemory = vertices.data();

        graphicsDevice->CreateBuffer(&vertexBufferDescription, &vertexInitializationData, &m_VertexBuffer_Position);
        /// Set name.


        // AABB
        // Vertex Buffer - Tangents
        // Skinning Buffers
        // Vertex Buffer - UV Set 0
        // Vertex Buffer - UV Set 1
        // Raytracing
        // Bindless Descriptors
    }
}