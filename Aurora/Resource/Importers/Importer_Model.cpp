#include "Aurora.h"
#include "Importer_Model.h"
#include "../Scene/World.h"
#include "../Scene/Entity.h"
#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Material.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

/*
    The importer will create our the mesh entity and add its components respectively. Hence, the interface frees up massively, allowing us to simply call Importer.Load("filePath"). In addition, 
    Load will inherently determine the type of model file we're loading in.
*/

using namespace Assimp;

namespace 
{
    const unsigned int ImportFlags =
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_PreTransformVertices |
        aiProcess_GenNormals |
        aiProcess_GenUVCoords |
        aiProcess_OptimizeMeshes |
        aiProcess_Debone |
        aiProcess_ValidateDataStructure;
}

MeshDerp::MeshDerp(const aiMesh* mesh)
{
    assert(mesh->HasPositions());
    assert(mesh->HasNormals());

    m_vertices.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < m_vertices.capacity(); ++i) {
        Vertex vertex;
        Aurora::RHI_Vertex_Position_UV_Normal newVertex;


        vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        newVertex.m_Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        newVertex.m_Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
            vertex.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
        }
        if (mesh->HasTextureCoords(0)) {
            vertex.texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            newVertex.m_UV = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        m_vertices.push_back(vertex);
        m_Vertices.push_back(newVertex);
    }

    m_faces.reserve(mesh->mNumFaces);
    for (size_t i = 0; i < m_faces.capacity(); ++i) 
    {
        assert(mesh->mFaces[i].mNumIndices == 3);
        m_faces.push_back({ mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] });
    }

    const uint32_t indexCount = mesh->mNumFaces * 3;
    std::vector<uint32_t> indices = std::vector<uint32_t>(indexCount);
    for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
    {
        aiFace& face = mesh->mFaces[faceIndex]; // Each face has 3 vertices.
        const unsigned int indicesIndex = (faceIndex * 3); // We grab the first index of each face.
        indices[indicesIndex + 0] = face.mIndices[0];  // Grab its face index.
        indices[indicesIndex + 1] = face.mIndices[1];  // Grab its face index 2.
        indices[indicesIndex + 2] = face.mIndices[2];  // Grab its face index 3.
    }

    m_Indices = indices;
}

std::shared_ptr<MeshDerp> MeshDerp::fromFile(const std::string& filename)
{
    // LogStream::initialize();

    std::printf("Loading mesh: %s\n", filename.c_str());

    std::shared_ptr<MeshDerp> mesh;
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filename, ImportFlags);
    if (scene && scene->HasMeshes()) {
        mesh = std::shared_ptr<MeshDerp>(new MeshDerp{ scene->mMeshes[0] });
    }
    else {
        throw std::runtime_error("Failed to load mesh file: " + filename);
    }
    return mesh;
}

std::shared_ptr<MeshDerp> MeshDerp::fromString(const std::string& data)
{
    // LogStream::initialize();

    std::shared_ptr<MeshDerp> mesh;
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFileFromMemory(data.c_str(), data.length(), ImportFlags, "nff");
    if (scene && scene->HasMeshes()) {
        mesh = std::shared_ptr<MeshDerp>(new MeshDerp{ scene->mMeshes[0] });
    }
    else {
        throw std::runtime_error("Failed to create mesh from string: " + data);
    }
    return mesh;
}

namespace Aurora
{
    Importer_Model::Importer_Model(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_WorldContext = m_EngineContext->GetSubsystem<World>();

        /// Get Version Information.
    }

    std::shared_ptr<Entity> Importer_Model::Load(const std::string& filePath, const std::string& fileName)
    {
        if (!FileSystem::Exists(filePath))
        {
            return nullptr;
        }

        // Model Parameters
        ModelParameters modelParameters;
        modelParameters.m_TriangleLimit = 1000000;
        modelParameters.m_VertexLimit = 1000000;
        modelParameters.m_MaxNormalSmoothingAngle = 80.0f;    // Normals exceeding this limit are not smoothed.
        modelParameters.m_MaxTangentSmoothingAngle = 80.0f;   // Tangents exceeding this limit are not smoothed. Default is 45, max is 175.
        modelParameters.m_FilePath = filePath;
        modelParameters.m_Name = fileName == "" ? FileSystem::GetFileNameWithoutExtensionFromFilePath(filePath) : fileName;

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

        std::shared_ptr<Entity> newEntity;

        // Read the 3D model file from disk.
        if (const aiScene* scene = importer.ReadFile(filePath, importerFlags))
        {
            /// Progress Tracking.
            modelParameters.m_AssimpScene = scene;
            modelParameters.m_HasAnimations = scene->mNumAnimations != 0;

            // Create root entity to match Assimp's root node.
            const bool isActive = true;
            newEntity = m_WorldContext->EntityCreate(isActive);
            if (fileName == "") { newEntity->SetName(modelParameters.m_Name); } else { newEntity->SetName(modelParameters.m_Name); }
            /// Set root entity.

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

        if (newEntity) { return newEntity; }
    }

    void Importer_Model::ParseNode(const aiNode* assimpNode, const ModelParameters& modelParameters, Entity* parentEntity, Entity* newEntity)
    {
        if (parentEntity) // Parent node has already been set.
        {
            newEntity->SetName(assimpNode->mName.C_Str());
        }

        /// Progress Tracking.

        // Set the transform of the parent node as the parent of the new entity's transform.
        const auto parentTransform = parentEntity ? parentEntity->GetTransform() : nullptr;
        newEntity->GetTransform()->SetParentTransform(parentTransform);
        
        
        /// Set the transformation matrix of the Assimp node to the new node.

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
                entity->GetTransform()->SetParentTransform(newEntity->GetTransform()); // Set parent.
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
            else
            {
                AURORA_ERROR("Object has no normals!");
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

        meshComponent->CreateRenderData();

        // Material
        if (modelParameters.m_AssimpScene->HasMaterials())
        {
            const auto assimpMaterial = modelParameters.m_AssimpScene->mMaterials[assimpMesh->mMaterialIndex];   // Get aiMaterial for this specific mesh.
            LoadMaterial(assimpMaterial, modelParameters, parentEntity);                                         // Convert it and add it to the model.
        }

        /// Bones.
    }

    void Importer_Model::LoadMaterial(aiMaterial* assimpMaterial, const ModelParameters& modelParameters, Entity* materialEntity)
    {
        if (!assimpMaterial)
        {
            AURORA_WARNING("One of the provided materials is null. LoadMaterial cannot be executed.");
            return;
        }

        Material* material = materialEntity->AddComponent<Material>();

        // Name
        aiString materialName;
        aiGetMaterialString(assimpMaterial, AI_MATKEY_NAME, &materialName);
        /// Set a resource file path so it can be used by the resource cache.

        // Diffuse Color
        aiColor4D diffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
        aiGetMaterialColor(assimpMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);

        // Opacity
        aiColor4D opacity(1.0f, 1.0f, 1.0f, 1.0f);
        aiGetMaterialColor(assimpMaterial, AI_MATKEY_OPACITY, &opacity);

        material->SetBaseColor( { diffuseColor.r, diffuseColor.g, diffuseColor.b, opacity.r } );

        const auto LoadMaterialTexture = [this, &modelParameters, &assimpMaterial, &material](TextureSlot engineSlotType, const aiTextureType assimpTextureTypePBR, const aiTextureType assimpTextureTypeLegacy)
        {
            aiTextureType typeAssimp = assimpMaterial->GetTextureCount(assimpTextureTypePBR) > 0 ? assimpTextureTypePBR : aiTextureType_NONE;
            typeAssimp = assimpMaterial->GetTextureCount(assimpTextureTypeLegacy) > 0 ? assimpTextureTypeLegacy : typeAssimp;

            aiString texturePath;
            if (assimpMaterial->GetTextureCount(typeAssimp) > 0)
            {
                if (AI_SUCCESS == assimpMaterial->GetTexture(typeAssimp, 0, &texturePath))
                {
                    const std::string deducedPath = ValidateTexturePath(texturePath.data, modelParameters.m_FilePath);
                    if (FileSystem::IsSupportedImageFile(deducedPath))
                    {
                        material->m_Textures[engineSlotType].m_FilePath = deducedPath;
                        material->m_Textures[engineSlotType].m_Resource = m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(deducedPath, FileSystem::GetFileNameFromFilePath(deducedPath));

                        if (typeAssimp == aiTextureType_BASE_COLOR || typeAssimp == aiTextureType_DIFFUSE)
                        {
                            // material->SetBaseColor({ 0.0f, 0.0f, 0.0f, 0.0f });
                        }

                        /// Normal/Height Map Stuff.

                        return; // Crucial, else we will revert to binding a default texture.
                    }
                }
            }

            AURORA_WARNING("Could not find texture of appropriate type. Binding default texture...");
            material->m_Textures[engineSlotType].m_Resource = m_EngineContext->GetSubsystem<Renderer>()->m_DefaultWhiteTexture;
            material->m_Textures[engineSlotType].m_FilePath = m_EngineContext->GetSubsystem<Renderer>()->m_DefaultWhiteTexture->m_FilePath;
        };
        
        // Engine Texture, Assimp PBR Texture, Assimp Legacy Texture (Fallback)
        LoadMaterialTexture(TextureSlot::BaseColorMap, aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE);
        LoadMaterialTexture(TextureSlot::RoughnessMap, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_SHININESS); // Use specular as fallback.
        LoadMaterialTexture(TextureSlot::MetalnessMap, aiTextureType_METALNESS, aiTextureType_AMBIENT); // Use ambient as fallback.
        LoadMaterialTexture(TextureSlot::NormalMap, aiTextureType_NORMAL_CAMERA, aiTextureType_NORMALS);
    }

    std::string Importer_Model::TextureTryMultipleExtensions(const std::string& filePath)
    {
        // Remove extension.
        const std::string filePathNoExtension = FileSystem::GetFilePathWithoutExtension(filePath);

        // Check if the file exists using all engine supported extensions.
        for (const std::string& supportedFormat : g_Supported_Image_Formats)
        {
            std::string newFilePath = filePathNoExtension + supportedFormat;
            std::string newFilePathUpper = filePathNoExtension + FileSystem::ConvertToUppercase(supportedFormat);

            if (FileSystem::Exists(newFilePath))
            {
                return newFilePath;
            }

            if (FileSystem::Exists(newFilePathUpper))
            {
                return newFilePathUpper;
            }
        }

        return filePath;
    }

    std::string Importer_Model::ValidateTexturePath(std::string originalTexturePath, const std::string& modelPath)
    {
        std::replace(originalTexturePath.begin(), originalTexturePath.end(), '\\', '/');

        // Models usually return a texture path that is relative to the model's directory. However, to load anything, we will need an absolute path. We will thus construct it here.
        const std::string modelDirectory = FileSystem::GetDirectoryFromFilePath(modelPath);
        std::string fullTexturePath = modelDirectory + originalTexturePath;

        // 1) Check if the texture path is valid.
        if (FileSystem::Exists(fullTexturePath))
        {
            AURORA_INFO("Model Texture Found: %s.", fullTexturePath.c_str());
            return fullTexturePath;
        }

        // 2) Check the same texture path as previously but this time with different file extensions (jpg, png and so on).
        fullTexturePath = TextureTryMultipleExtensions(fullTexturePath);
        if (FileSystem::Exists(fullTexturePath))
        {
            AURORA_INFO("Model Texture Found: %s.", fullTexturePath.c_str());
            return fullTexturePath;
        }

        // At this point, we know that the provided path is wrong, but we can make a few guesses. The most common mistake is that the arist provided a path relative to his/her computer.
        // 3) Check if the texture is in the same folder as the model.
        fullTexturePath = modelDirectory + FileSystem::GetFileNameFromFilePath(fullTexturePath);
        if (FileSystem::Exists(fullTexturePath))
        {
            AURORA_INFO("Model Texture Found: %s.", fullTexturePath.c_str());
            return fullTexturePath;
        }

        // 4) Check the same texture path as previously but with different file extensions (jpg, png and so on).
        fullTexturePath = TextureTryMultipleExtensions(fullTexturePath);
        if (FileSystem::Exists(fullTexturePath))
        {
            AURORA_INFO("Model Texture Found: %s.", fullTexturePath.c_str());
            return fullTexturePath;
        }

        // Give up, no valid texture path was found.
        AURORA_WARNING("Model Texture could not be found: %s.", fullTexturePath.c_str());
        return "";
    }

 /* Old Model Loading with TinyOBJ
 
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
 */
}