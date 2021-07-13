#include "Aurora.h"
#include "Mesh.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    Mesh::Mesh(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

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

            //graphicsDevice->CreateBuffer(&indexBufferDescription, &indexInitializationData, &m_MeshComponent.m_IndexBuffer);
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

            //graphicsDevice->CreateBuffer(&indexBufferDescription, &indexInitializationData, &m_MeshComponent.m_IndexBuffer);
            /// Set name.
        }

        // For AABB in the future.
        XMFLOAT3 minimumVector = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
        XMFLOAT3 maximumVector = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // Grab vertex positions and store them in our own data structure.
        std::vector<Vertex_Position> vertices(m_VertexPositions.size());
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            const XMFLOAT3& position = m_VertexPositions[i];
            /// Normals.
            /// Wind Weights.
            vertices[i].Populate(position, m_UVSet_0[i], m_VertexNormals[i]);

            minimumVector = Aurora::Math::Minimum(minimumVector, position);
            maximumVector = Aurora::Math::Maximum(maximumVector, position);
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