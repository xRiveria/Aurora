#include "Aurora.h"
#include "Importer_Model.h"
#include "../Scene/World.h"
#include "../Scene/Entity.h"
#include "../Scene/Components/Mesh.h"

#pragma warning(push, 0)
#define TINYOBJLOADER_IMPLEMENTATION
#include "../Resource/Importers/tiny_obj_loader/tiny_obj_loader.h"
#pragma warning(pop)

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

/*
    The importer will create our the mesh entity and add its components respectively. Hence, the interface frees up massively, allowing us to simply call Importer.Load("filePath"). In addition, 
    Load will inherently determine the type of model file we're loading in.
*/

using namespace Assimp;

namespace Aurora
{
    Importer_Model::Importer_Model(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_WorldContext = m_EngineContext->GetSubsystem<World>();

        /// Get Version Information.
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
            ImporterModel_General(filePath);
        }
        else if (!extension.compare(".GLB"))
        {

        }
    }

    bool Importer_Model::ImporterModel_General(const std::string& filePath)
    {
        if (!FileSystem::Exists(filePath))
        {
            return false;
        }

        // Model Parameters
        ModelParameters modelParameters;
        modelParameters.m_TriangleLimit = 1000000;
        modelParameters.m_VertexLimit = 1000000;
        modelParameters.m_MaxNormalSmoothingAngle = 80.0f;    // Normals exceeding this limit are not smoothed.
        modelParameters.m_MaxTangentSmoothingAngle = 80.0f;   // Tangents exceeding this limit are not smoothed. Default is 45, max is 175.
        modelParameters.m_FilePath = filePath;
        modelParameters.m_Name = FileSystem::GetNameFromFilePath(filePath);

        // Setup an Assimp Importer.
        Importer importer;
        
        const uint32_t importerFlags =
            aiProcess_MakeLeftHanded            |
            aiProcess_FlipUVs                   |
            aiProcess_FlipWindingOrder          |
            aiProcess_CalcTangentSpace          |
            aiProcess_GenSmoothNormals          |
            aiProcess_JoinIdenticalVertices     |
            aiProcess_OptimizeMeshes            |
            aiProcess_ImproveCacheLocality      |
            aiProcess_RemoveRedundantMaterials  |
            aiProcess_LimitBoneWeights          |
            aiProcess_SplitLargeMeshes          |
            aiProcess_Triangulate               |
            aiProcess_GenUVCoords               |
            aiProcess_SortByPType               |
            aiProcess_FindDegenerates           |
            aiProcess_FindInvalidData           |
            aiProcess_FindInstances             |
            aiProcess_ValidateDataStructure     |
            aiProcess_Debone;

        // Read the 3D model file from disk.
        if (const aiScene* scene = importer.ReadFile(filePath, importerFlags))
        {
            /// Progress Tracking.
            modelParameters.m_AssimpScene = scene;
            modelParameters.m_HasAnimations = scene->mNumAnimations != 0;

            // Create root entity to match Assimp's root node.
            const bool isActive = false;
            std::shared_ptr<Entity> newEntity = m_WorldContext->EntityCreate(isActive);
            newEntity->SetName(modelParameters.m_Name); // Set custom name, which is more descriptive than "RootNode".
            /// Set root entity for model.

            // Parse all nodes, starting from the root node and continuing recursively.
            ParseNode(scene->mRootNode, modelParameters, nullptr, newEntity.get());
            /// Parse Animations.

            // Update model geometry.
            /// Create Render Data.
        }
        else
        {
            AURORA_ERROR("Failed to load model at %s", filePath.c_str());
        }

        importer.FreeScene();

        return modelParameters.m_AssimpScene != nullptr;
    }

    void Importer_Model::ParseNode(const aiNode* assimpNode, const ModelParameters& modelParameters, Entity* parentEntity, Entity* newEntity)
    {
    }
    void Importer_Model::ParseNodeMeshes(const aiNode* assimpNode, Entity& newEntity, const ModelParameters& modelParameters)
    {
    }
    void Importer_Model::ParseAnimations(const ModelParameters& modelParameters)
    {
    }
    void Importer_Model::LoadMesh(aiMesh* assimpMesh, Entity* parentEntity, const ModelParameters& modelParameters)
    {
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