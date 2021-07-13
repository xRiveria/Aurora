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

std::string g_TemporaryPath = "Derp";

namespace Aurora
{
    Importer_Model::Importer_Model(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_WorldContext = m_EngineContext->GetSubsystem<World>();

        /// Get Version Information.
    }

    void Importer_Model::Load(const std::string& filePath, const std::string& albedoPath)
    {

        ImporterModel_General(filePath, albedoPath);
    }

    bool Importer_Model::ImporterModel_General(const std::string& filePath, const std::string& albedoPath)
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
        // Maximum number of triangles in a mesh (before splitting)    
        importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, modelParameters.m_TriangleLimit);
        // Maximum number of vertices in a mesh (before splitting)    
        importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, modelParameters.m_VertexLimit);
        // Remove points and lines.
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
        // Remove cameras and lights
        importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);
        
        const uint32_t importerFlags =
            aiProcess_Triangulate               |
            aiProcess_MakeLeftHanded            |
            aiProcess_FlipUVs                   |
            aiProcess_JoinIdenticalVertices     |
            aiProcess_FindDegenerates           |
            aiProcess_SortByPType               |
            // aiProcess_FlipWindingOrder          |
            aiProcess_CalcTangentSpace          |
            aiProcess_GenSmoothNormals          |
            aiProcess_OptimizeMeshes            |
            aiProcess_ImproveCacheLocality      |
            aiProcess_RemoveRedundantMaterials  |
            aiProcess_LimitBoneWeights          |
            aiProcess_SplitLargeMeshes          |
            aiProcess_GenUVCoords               |
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
            g_TemporaryPath = albedoPath;
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
        if (parentEntity) // Parent node has already been set.
        {
            newEntity->SetName(assimpNode->mName.C_Str());
        }

        /// Progress Tracking.

        /// Transform Stuff.

        // Process all of the node's meshes.
        ParseNodeMeshes(assimpNode, newEntity, modelParameters);

        // Process children.
        for (uint32_t i = 0; i < assimpNode->mNumChildren; i++)
        {
            std::shared_ptr<Entity> child = m_WorldContext->EntityCreate();
            ParseNode(assimpNode->mChildren[i], modelParameters, newEntity, child.get()); // Our new entity becomes the parent.
        }

        /// Progress Tracking.
    }


    void Importer_Model::ParseNodeMeshes(const aiNode* assimpNode, Entity* newEntity, const ModelParameters& modelParameters)
    {
        for (uint32_t i = 0; i < assimpNode->mNumMeshes; i++)
        {
            Entity* entity = newEntity; // Set the current entity.
            const auto assimpMesh = modelParameters.m_AssimpScene->mMeshes[assimpNode->mMeshes[i]]; // Retrieve mesh.
            std::string meshName = assimpNode->mName.C_Str(); // Retrieve name of the mesh.

            // If this node has multiple meshes, we will create and assign a new entity for each of them.
            if (assimpNode->mNumMeshes > 1)
            {
                const bool isActive = false;
                entity = m_WorldContext->EntityCreate(isActive).get(); // Create entity.
                /// Set parent transform.
                meshName += "_" + std::to_string(i + 1); // Set name.
            }

            // Set entity name.
            entity->SetName(meshName);

            // Process Mesh.
            LoadMesh(assimpMesh, entity, modelParameters);

            entity->SetActive(true);
        }
    }

    void Importer_Model::LoadMesh(aiMesh* assimpMesh, Entity* parentEntity, const ModelParameters& modelParameters)
    {
        if (!assimpMesh || !parentEntity)
        {
            AURORA_ERROR_INVALID_PARAMETER();
            return;
        }

        const uint32_t vertexCount = assimpMesh->mNumVertices;
        const uint32_t indexCount = assimpMesh->mNumFaces * 3;

        Mesh* meshComponent = parentEntity->AddComponent<Mesh>();

        // Load objects, meshes. Each of our shapes (meshes) is created as a seperate entity in our architecture.  
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            XMFLOAT3 storedPosition;
            XMFLOAT3 storedNormal;
            XMFLOAT2 storedTextureCoordinates;

            // Position
            const auto& position = assimpMesh->mVertices[i];
            storedPosition.x = position.x;
            storedPosition.y = position.y;
            storedPosition.z = position.z;

            meshComponent->m_VertexPositions.push_back(storedPosition);

            // Normal
            if (assimpMesh->mNormals)
            {
                const auto& normal = assimpMesh->mNormals[i];
                storedNormal.x = normal.x;
                storedNormal.y = normal.y;
                storedNormal.z = normal.z;

                meshComponent->m_VertexNormals.push_back(storedNormal);
            }

            // Texture Coordinates
            const uint32_t uvChannel = 0;
            if (assimpMesh->HasTextureCoords(uvChannel))
            {
                const auto& texCoords = assimpMesh->mTextureCoords[uvChannel][i];
                storedTextureCoordinates.x = texCoords.x;
                storedTextureCoordinates.y = texCoords.y;

                meshComponent->m_UVSet_0.push_back(storedTextureCoordinates);
            }
        }

        // Indices
        std::vector<uint32_t> indices = std::vector<uint32_t>(indexCount);
        // Get indices by iterating through each face of the mesh.
        for (uint32_t faceIndex = 0; faceIndex < assimpMesh->mNumFaces; faceIndex++)
        {
            aiFace& face = assimpMesh->mFaces[faceIndex];
            const unsigned int indicesIndex = (faceIndex * 3);
            indices[indicesIndex + 0] = face.mIndices[0];
            indices[indicesIndex + 1] = face.mIndices[1];
            indices[indicesIndex + 2] = face.mIndices[2];
        }
        
        meshComponent->m_Indices = indices;
        meshComponent->m_BaseTexture = m_EngineContext->GetSubsystem<ResourceCache>()->Load("Hollow_Knight_Albedo.png", g_TemporaryPath);

        meshComponent->CreateRenderData();
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