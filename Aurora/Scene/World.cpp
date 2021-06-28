#include "Aurora.h"
#include "World.h"
#include "../Core/FileSystem.h"
#include <type_traits>

#pragma warning(push, 0)
#define TINYOBJLOADER_IMPLEMENTATION
#include "../Resource/Importers/tiny_obj_loader/tiny_obj_loader.h"
#pragma warning(pop)

using namespace DirectX;

namespace Aurora
{
    World::World(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    World::~World()
    {

    }

    void World::LoadModel(const std::string& filePath)
    {
        std::string extension = FileSystem::ConvertToUppercase(FileSystem::GetExtensionFromFilePath(filePath));

        // For now, we will just output whatever we upload into the active scene.
        if (!extension.compare(".OBJ"))
        {
            ImportModel_OBJ(filePath);
        }
        else if (!extension.compare(".GLTF"))
        {

        }
        else if (!extension.compare(".GLB"))
        {

        }
    }

    void World::ImportModel_OBJ(const std::string& filePath)
    {
        // Transform the data from OBJ space to engine-space.
        static const bool transformToLeftHanded = true;
        bool success = false;

        tinyobj::attrib_t object_Attributes;
        std::vector<tinyobj::shape_t> object_Shapes;
        std::vector<tinyobj::material_t> object_Materials;

        std::string objectLoadErrors;
        std::string objectLoadWarnings;

        success = tinyobj::LoadObj(&object_Attributes, &object_Shapes, &object_Materials, &objectLoadWarnings, &objectLoadErrors, filePath.c_str());

        if (success)
        {
            if (!objectLoadWarnings.empty())
            {
                AURORA_WARNING(objectLoadWarnings);
            }

            if (!objectLoadErrors.empty())
            {
                AURORA_ERROR(objectLoadErrors);
            }

            AURORA_INFO("Loaded model at path: %s.", filePath.c_str());

            m_MeshComponent.m_BaseTexture = m_EngineContext->GetSubsystem<ResourceCache>()->Load("Hollow_Knight_Albedo.png", "../Resources/Models/Hollow_Knight/textures/None_2_Base_Color.png");
        }
        else
        {
            AURORA_ERROR("Failed to load model at path: %s.", filePath.c_str());
            return;
        }

        // Load objects, meshes. Each of our shapes (meshes) is created as a seperate entity in our architecture.
        for (tinyobj::shape_t& shape : object_Shapes)
        {
            // Create Object.
            // 
            // Create Mesh
            std::unordered_map<size_t, uint32_t> uniqueVertices = {};

            for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
            {
                tinyobj::index_t reorderedIndices[] =
                {
                    shape.mesh.indices[i + 0],
                    shape.mesh.indices[i + 1],
                    shape.mesh.indices[i + 2]
                };

                bool flipCulling = true;
                if (flipCulling)
                {
                    reorderedIndices[1] = shape.mesh.indices[i + 2];
                    reorderedIndices[2] = shape.mesh.indices[i + 1];
                }

                for (auto& index : reorderedIndices)
                {
                    XMFLOAT3 position = XMFLOAT3(
                        object_Attributes.vertices[index.vertex_index * 3 + 0],
                        object_Attributes.vertices[index.vertex_index * 3 + 1],
                        object_Attributes.vertices[index.vertex_index * 3 + 2]
                    );

                    XMFLOAT2 texCoords = XMFLOAT2(0, 0);
                    if (index.texcoord_index >= 0 && !object_Attributes.texcoords.empty())
                    {
                        texCoords = XMFLOAT2(
                            object_Attributes.texcoords[index.texcoord_index * 2 + 0],
                            1 - object_Attributes.texcoords[index.texcoord_index * 2 + 1]
                        );
                    }

                    XMFLOAT3 normals = XMFLOAT3(0, 0, 0);
                    if (!object_Attributes.normals.empty())
                    {
                        normals = XMFLOAT3(
                            object_Attributes.normals[index.normal_index * 3 + 0],
                            object_Attributes.normals[index.normal_index * 3 + 1],
                            object_Attributes.normals[index.normal_index * 3 + 2]
                        );
                    }

                    /// Material Indexing.

                    if (transformToLeftHanded)
                    {
                        position.z *= -1;
                        normals.z  *= -1;
                    }

                    m_MeshComponent.m_VertexPositions.push_back(position);
                    m_MeshComponent.m_UVSet_0.push_back(texCoords);
                    m_MeshComponent.m_VertexNormals.push_back(normals);

                    // Eliminate duplicate vertices by means of hashing.
                    // size_t vertexHash = 0;
                    // Aurora::Utilities::Hash_Combine(vertexHash, index.vertex_index);

                    // if (uniqueVertices.count(vertexHash) == 0)
                    // {
                    // uniqueVertices[vertexHash] = (uint32_t)m_MeshComponent.m_VertexPositions.size();

                        
                    // }

                    // m_MeshComponent.m_Indices.push_back(uniqueVertices[vertexHash]);
                }
            }

            CreateRenderData();
        }
    }

    // Having some problems putting this in our mesh component. We will put this function into our world for now.
    void World::CreateRenderData()
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
        if (m_MeshComponent.GetIndexFormat() == IndexBuffer_Format::Format_32Bit)
        {
            indexBufferDescription.m_StructureByteStride = sizeof(uint32_t);
            indexBufferDescription.m_Format = Format::FORMAT_R32_UINT;
            indexBufferDescription.m_ByteWidth = uint32_t(sizeof(uint32_t) * m_MeshComponent.m_Indices.size());

            // Use indices directly since vector is in correct format.
            static_assert(std::is_same<decltype(m_MeshComponent.m_Indices)::value_type, uint32_t>::value, "Indices not in Index_Format::32_Bit.");
            indexInitializationData.m_SystemMemory = m_MeshComponent.m_Indices.data();

            //graphicsDevice->CreateBuffer(&indexBufferDescription, &indexInitializationData, &m_MeshComponent.m_IndexBuffer);
            /// Set name.
        }
        // 16 Bit
        else
        {
            indexBufferDescription.m_StructureByteStride = sizeof(uint16_t);
            indexBufferDescription.m_Format = Format::FORMAT_R16_UINT;
            indexBufferDescription.m_ByteWidth = uint32_t(sizeof(uint16_t) * m_MeshComponent.m_Indices.size());

            std::vector<uint16_t> gpuIndexBuffer(m_MeshComponent.m_Indices.size()); // 16 bit buffer.
            std::copy(m_MeshComponent.m_Indices.begin(), m_MeshComponent.m_Indices.end(), gpuIndexBuffer.begin());
            indexInitializationData.m_SystemMemory = gpuIndexBuffer.data();

            //graphicsDevice->CreateBuffer(&indexBufferDescription, &indexInitializationData, &m_MeshComponent.m_IndexBuffer);
            /// Set name.
        }

        // For AABB in the future.
        XMFLOAT3 minimumVector = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
        XMFLOAT3 maximumVector = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // Grab vertex positions and store them in our own data structure.
        std::vector<Vertex_Position> vertices(m_MeshComponent.m_VertexPositions.size());
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            const XMFLOAT3& position = m_MeshComponent.m_VertexPositions[i];
            /// Normals.
            /// Wind Weights.
            vertices[i].Populate(position, m_MeshComponent.m_UVSet_0[i], m_MeshComponent.m_VertexNormals[i]);
             
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

        graphicsDevice->CreateBuffer(&vertexBufferDescription, &vertexInitializationData, &m_MeshComponent.m_VertexBuffer_Position);
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