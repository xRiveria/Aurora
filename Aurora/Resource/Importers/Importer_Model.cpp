#include "Aurora.h"
#include "Importer_Model.h"
#include "../Scene/World.h"
#include "../Scene/Entity.h"
#include "../Scene/Components/Mesh.h"

#pragma warning(push, 0)
#define TINYOBJLOADER_IMPLEMENTATION
#include "../Resource/Importers/tiny_obj_loader/tiny_obj_loader.h"
#pragma warning(pop)

/*
    The importer will create our the mesh entity and add its components respectively. Hence, the interface frees up massively, allowing us to simply call Importer.Load("filePath"). In addition, 
    Load will inherently determine the type of model file we're loading in.
*/

namespace Aurora
{
    Importer_Model::Importer_Model(EngineContext* engineContext) : m_EngineContext(engineContext)
    {

    }

    void Importer_Model::Load(const std::string& filePath, const std::string& albedoPath)
    {
        std::string extension = FileSystem::ConvertToUppercase(FileSystem::GetExtensionFromFilePath(filePath));

        if (!extension.compare(".OBJ"))
        {
            ImportModel_OBJ(filePath, albedoPath);
        }
        else if (!extension.compare(".GLTF"))
        {

        }
        else if (!extension.compare(".GLB"))
        {

        }
    }

    void Importer_Model::ImportModel_OBJ(const std::string& filePath, const std::string& albedoPath)
    {
        World* worldContext = m_EngineContext->GetSubsystem<World>();

        // Automatically create an Entity and add a mesh component.
        Entity* entity = worldContext->EntityCreate().get();
        Mesh* meshComponent = entity->AddComponent<Mesh>();

        meshComponent->m_BaseTexture = m_EngineContext->GetSubsystem<ResourceCache>()->Load("Hollow_Knight_Albedo.png", albedoPath); // Temporary.

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
            entity->SetName(shape.name + "_Mesh");
            std::unordered_map<size_t, uint32_t> uniqueVertices = {};

            for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
            {
                tinyobj::index_t reorderedIndices[] =
                {
                    shape.mesh.indices[i + 0],
                    shape.mesh.indices[i + 1],
                    shape.mesh.indices[i + 2]
                };

                bool flipCulling = false;
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
                        normals.z *= -1;
                    }

                    meshComponent->m_VertexPositions.push_back(position);
                    meshComponent->m_UVSet_0.push_back(texCoords);
                    meshComponent->m_VertexNormals.push_back(normals);

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

            meshComponent->CreateRenderData();
        }
    }
}