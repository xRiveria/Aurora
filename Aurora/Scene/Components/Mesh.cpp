#include "Aurora.h"
#include "Mesh.h"

namespace Aurora
{
    Mesh::Mesh(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    void Mesh::CreateRenderData()
    {
        DX11_GraphicsDevice* graphicsDevice = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice.get();

        // Create Index Buffer GPU Data.
        {
            RHI_GPU_Buffer_Description bufferDescription;
            bufferDescription.m_Usage = Usage::Immutable;
            bufferDescription.m_CPUAccessFlags = 0;
            bufferDescription.m_BindFlags = Bind_Flag::Bind_Index_Buffer | Bind_Flag::Bind_Shader_Resource;
            bufferDescription.m_MiscFlags = 0;

            RHI_Subresource_Data initializationData;

            if (GetIndexFormat() == IndexBuffer_Format::Format_32Bit)
            {
                bufferDescription.m_StructureByteStride = sizeof(uint32_t);
                bufferDescription.m_Format = Format::FORMAT_R32_UINT;
                bufferDescription.m_ByteWidth = uint32_t(sizeof(uint32_t) * m_Indices.size());

                // Use indices directly since vector is in correct format.
                static_assert(std::is_same<decltype(m_Indices)::value_type, uint32_t>::value, "Indices not in Index Format 32 Bit");
                initializationData.m_SystemMemory = m_Indices.data();

                graphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &m_Index_Buffer);
                /// Set Name.
            }
            else
            {
                bufferDescription.m_StructureByteStride = sizeof(uint16_t);
                bufferDescription.m_Format = Format::FORMAT_R16_UINT;
                bufferDescription.m_ByteWidth = uint32_t(sizeof(uint16_t) * m_Indices.size());

                // Convert to a uint16_t buffer to save some memory and bandwidth.
                std::vector<uint16_t> gpuIndexData(m_Indices.size());
                std::copy(m_Indices.begin(), m_Indices.end(), gpuIndexData.begin());

                initializationData.m_SystemMemory = gpuIndexData.data();

                graphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &m_Index_Buffer);
                /// Set Name.
            }
        }

        XMFLOAT3 minimumVector = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
        XMFLOAT3 maximumVector = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // Vertex Buffer - Position + Normal + Wind
        {
            std::vector<Vertex_Position> vertices(m_Vertex_Positions.size());
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                const XMFLOAT3& position = m_Vertex_Positions[i];
                XMFLOAT3 normal = m_Vertex_Normals.empty() ? XMFLOAT3(1, 1, 1) : m_Vertex_Normals[i];
                XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&normal))); // Normalize normals. We will only need their directions after all. 
                const uint8_t wind = m_Vertex_WindWeights.empty() ? 0xFF : m_Vertex_WindWeights[i];
                vertices[i].PopulateFromParameters(position, normal, wind);

                // AABB
                minimumVector = Math::Minimum(minimumVector, position);
                maximumVector = Math::Maximum(maximumVector, position);
            }

            RHI_GPU_Buffer_Description bufferDescription;
            bufferDescription.m_Usage = Usage::Default;
            bufferDescription.m_CPUAccessFlags = 0;
            bufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer | Bind_Flag::Bind_Shader_Resource;
            bufferDescription.m_MiscFlags = Resource_Misc_Flag::Resource_Misc_Buffer_Allow_Raw_Views;
            /// Raytracing Misc.
            bufferDescription.m_ByteWidth = (uint32_t)(sizeof(Vertex_Position) * vertices.size());

            RHI_Subresource_Data initializationData;
            initializationData.m_SystemMemory = vertices.data();
            graphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &m_Vertex_Buffer_Position);
            /// Set Name.
        }

        // Vertex Buffer - Tangents
        if (!m_Vertex_Tangents.empty())
        {
            /// Automatically generate?

            std::vector<Vertex_Tangent> vertices(m_Vertex_Tangents.size());
            for (size_t i = 0; i < m_Vertex_Tangents.size(); ++i)
            {
                vertices[i].PopulateFromParameters(m_Vertex_Tangents[i]);
            }

            RHI_GPU_Buffer_Description bufferDescription;
            bufferDescription.m_Usage = Usage::Default;
            bufferDescription.m_CPUAccessFlags = 0;
            bufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer | Bind_Flag::Bind_Shader_Resource;
            bufferDescription.m_MiscFlags = Resource_Misc_Flag::Resource_Misc_Buffer_Allow_Raw_Views;
            bufferDescription.m_StructureByteStride = sizeof(Vertex_Tangent);
            bufferDescription.m_ByteWidth = (uint32_t)(bufferDescription.m_StructureByteStride * vertices.size());

            RHI_Subresource_Data initializationData;
            initializationData.m_SystemMemory = vertices.data();

            graphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &m_Vertex_Buffer_Tangent);
            /// Set Name.
        }

        /// AABB Creation.

        /// Skinning Buffers.

        // Vertex Buffer - UV Set 0
        if (!m_Vertex_UVSet_0.empty())
        {
            std::vector<Vertex_TexCoord> vertices(m_Vertex_UVSet_0.size());
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                vertices[i].PopulateFromParameters(m_Vertex_UVSet_0[i]);
            }

            RHI_GPU_Buffer_Description bufferDescription;
            bufferDescription.m_Usage = Usage::Immutable;
            bufferDescription.m_CPUAccessFlags = 0;
            bufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer | Bind_Flag::Bind_Shader_Resource;
            bufferDescription.m_MiscFlags = Resource_Misc_Flag::Resource_Misc_Buffer_Allow_Raw_Views;
            bufferDescription.m_StructureByteStride = sizeof(Vertex_TexCoord);
            bufferDescription.m_ByteWidth = (uint32_t)(bufferDescription.m_StructureByteStride * vertices.size());

            RHI_Subresource_Data initializationData;
            initializationData.m_SystemMemory = vertices.data();

            graphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &m_Vertex_Buffer_UV0);
            /// Set Name.
        }

        // Vertex Buffer - UV Set 1
        if (!m_Vertex_UVSet_1.empty())
        {
            std::vector<Vertex_TexCoord> vertices(m_Vertex_UVSet_1.size());
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                vertices[i].PopulateFromParameters(m_Vertex_UVSet_1[i]);
            }

            RHI_GPU_Buffer_Description bufferDescription;
            bufferDescription.m_Usage = Usage::Immutable;
            bufferDescription.m_CPUAccessFlags = 0;
            bufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer | Bind_Flag::Bind_Shader_Resource;
            bufferDescription.m_MiscFlags = Resource_Misc_Flag::Resource_Misc_Buffer_Allow_Raw_Views;
            bufferDescription.m_StructureByteStride = sizeof(Vertex_TexCoord);
            bufferDescription.m_ByteWidth = (uint32_t)(bufferDescription.m_StructureByteStride * vertices.size());

            RHI_Subresource_Data initializationData;
            initializationData.m_SystemMemory = vertices.data();

            graphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &m_Vertex_Buffer_UV1);
            /// Set Name.
        }

        // Vertex Buffer - Colors
        if (!m_Vertex_Colors.empty())
        {
            RHI_GPU_Buffer_Description bufferDescription;
            bufferDescription.m_Usage = Usage::Immutable;
            bufferDescription.m_CPUAccessFlags = 0;
            bufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer | Bind_Flag::Bind_Shader_Resource;
            bufferDescription.m_MiscFlags = Resource_Misc_Flag::Resource_Misc_Buffer_Allow_Raw_Views;
            bufferDescription.m_StructureByteStride = sizeof(Vertex_Color);
            bufferDescription.m_ByteWidth = (uint32_t)(bufferDescription.m_StructureByteStride * m_Vertex_Colors.size());

            RHI_Subresource_Data initializationData;
            initializationData.m_SystemMemory = m_Vertex_Colors.data();

            graphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &m_Vertex_Buffer_Color);
            /// Set Name.
        }

        /// Atlas.

        // Vertex Buffer - Subsets
        {
            m_Vertex_Subsets.resize(m_Vertex_Positions.size());

            uint32_t subsetCounter = 0;
            for (MeshSubset& subset : m_Subsets)
            {
                for (uint32_t i = 0; i < subset.m_Index_Count; ++i)
                {
                    uint32_t index = m_Indices[subset.m_Index_Offset + i];
                    m_Vertex_Subsets[index] = subsetCounter;
                }

                subsetCounter++;
            }

            RHI_GPU_Buffer_Description bufferDescription;
            bufferDescription.m_Usage = Usage::Immutable;
            bufferDescription.m_CPUAccessFlags = 0;
            bufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer | Bind_Flag::Bind_Shader_Resource;
            bufferDescription.m_MiscFlags = 0;
            bufferDescription.m_StructureByteStride = sizeof(uint8_t);
            bufferDescription.m_ByteWidth = (uint32_t)(bufferDescription.m_StructureByteStride * m_Vertex_Subsets.size());
            bufferDescription.m_Format = Format::FORMAT_R8_UINT;

            RHI_Subresource_Data initializationData;
            initializationData.m_SystemMemory = m_Vertex_Subsets.data();

            graphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &m_Vertex_Buffer_Subsets);
            /// Set Name.
        }

        /// Pre.
        /// Raytracing, Bindless Descriptors.
    }

    void Mesh::ComputeNormals(Compute_Normals_Type computeType)
    {
        // We will be utilizing hard normals for now.
        return;
    }

    void Mesh::FlipCulling()
    {
        for (size_t face = 0; face < m_Indices.size() / 3; face++)
        {
            uint32_t i0 = m_Indices[face * 3 + 0];
            uint32_t i1 = m_Indices[face * 3 + 1];
            uint32_t i2 = m_Indices[face * 3 + 2];

            // Take note of winding order. Our engine is using clockwise winding order.
            m_Indices[face * 3 + 0] = i0;
            m_Indices[face * 3 + 1] = i2;
            m_Indices[face * 3 + 2] = i1;
        }

        CreateRenderData();
    }

    void Mesh::FlipNormals()
    {
        for (XMFLOAT3& normal : m_Vertex_Normals)
        {
            normal.x *= -1;
            normal.y *= -1;
            normal.z *= -1;
        }

        CreateRenderData(); // Recreate render data after flipping our normals.
    }
}