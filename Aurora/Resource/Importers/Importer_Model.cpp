#include "Aurora.h"
#include "Importer_Model.h"
#include "../Renderer/Renderer.h"
#include "../Scene/World.h"
#include "../Scene/Entity.h"
#include "../Resource/ResourceCache.h"
#include "../Renderer/Model.h"
#include "../Renderer/Mesh.h"
#include "../Scene/Components/Renderable.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "../Resource/AuroraResource.h"

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

    bool Importer_Model::LoadModel(const std::string& filePath, Model* model)
    {
        AURORA_ASSERT(model != nullptr);

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
        modelParameters.m_Name = FileSystem::GetFileNameWithoutExtensionFromFilePath(filePath);
        modelParameters.m_Model = model;

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
            aiProcess_Triangulate |
            aiProcess_MakeLeftHanded |
            aiProcess_FlipUVs |
            aiProcess_JoinIdenticalVertices |
            aiProcess_FindDegenerates |
            aiProcess_SortByPType |
            // aiProcess_FlipWindingOrder          |
            aiProcess_CalcTangentSpace |
            aiProcess_GenSmoothNormals |
            aiProcess_OptimizeMeshes |
            aiProcess_ImproveCacheLocality |
            aiProcess_RemoveRedundantMaterials |
            aiProcess_LimitBoneWeights |
            aiProcess_SplitLargeMeshes |
            aiProcess_GenUVCoords |
            aiProcess_FindDegenerates |
            aiProcess_FindInvalidData |
            aiProcess_FindInstances |
            aiProcess_ValidateDataStructure |
            aiProcess_Debone;

        // Read the 3D model file from disk.
        if (const aiScene* scene = importer.ReadFile(filePath, importerFlags))
        {
            /// Progress Tracking.
            modelParameters.m_AssimpScene = scene;
            modelParameters.m_HasAnimations = scene->mNumAnimations != 0;

            // Create root entity to match Assimp's root node.
            const bool isActive = true;
            std::shared_ptr<Entity> newEntity = m_WorldContext->EntityCreate(isActive);
            newEntity->SetEntityName(modelParameters.m_Name);
            modelParameters.m_Model->SetRootEntity(newEntity);

            // Parse all nodes, starting from the root node and continuing recursively.
            ParseNode(scene->mRootNode, modelParameters, nullptr, newEntity.get());

            /// Parse Animations.

            // Update model geometry. In the future, we would create this function in our Model class for more complex behavior, such as modifying bounding boxes.
            // For now, we will simply create our buffers here.
            modelParameters.m_Model->CreateBuffers();
        }
        else
        {
            AURORA_ERROR(LogLayer::Engine, "Failed to load model at %s", filePath.c_str());
        }

        importer.FreeScene();

        return modelParameters.m_AssimpScene != nullptr;
    }

    void Importer_Model::ParseNode(const aiNode* assimpNode, const ModelParameters& modelParameters, Entity* parentEntity, Entity* newEntity)
    {
        if (parentEntity) // Parent node has already been set.
        {
            newEntity->SetEntityName(assimpNode->mName.C_Str());
        }

        /// Progress Tracking.

        // Set the transform of the parent node as the parent of the new entity's transform.
        Transform* parentTransform = parentEntity ? parentEntity->GetTransform() : nullptr;
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
            entity->SetEntityName(meshName);

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

        // What we are attempting to retrieve from the model:
        std::vector<XMFLOAT3> vertexPositions = std::vector<XMFLOAT3>(vertexCount);
        std::vector<XMFLOAT3> vertexNormals = std::vector<XMFLOAT3>(vertexCount);
        std::vector<XMFLOAT2> vertexUVs = std::vector<XMFLOAT2>(vertexCount);

        // Vertices
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            // Position
            const auto& position = assimpMesh->mVertices[i];
            vertexPositions[i].x = position.x;
            vertexPositions[i].y = position.y;
            vertexPositions[i].z = position.z;

            // Normal
            if (assimpMesh->mNormals)
            {
                const auto& normal = assimpMesh->mNormals[i];
                vertexNormals[i].x = normal.x;
                vertexNormals[i].y = normal.y;
                vertexNormals[i].z = normal.z;
            }

            // Texture Coordinates
            const uint32_t uvChannel = 0;
            if (assimpMesh->HasTextureCoords(uvChannel))
            {
                const auto& texCoords = assimpMesh->mTextureCoords[uvChannel][i];
                vertexUVs[i].x = texCoords.x;
                vertexUVs[i].y = texCoords.y;
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

        /// Compute AABB
        // Add the mesh to the model. 
        uint32_t indexOffset;
        uint32_t vertexOffset;
        modelParameters.m_Model->AppendGeometry(std::move(indices), std::move(vertexPositions), std::move(vertexNormals), std::move(vertexUVs), &indexOffset, &vertexOffset);

        // Add a renderable component to the entity.
        Renderable* renderable = parentEntity->AddComponent<Renderable>();

        // Set the geometry within our renderable component.
        renderable->GeometrySet(parentEntity->GetObjectName(), indexOffset, static_cast<uint32_t>(indices.size()), vertexOffset, static_cast<uint32_t>(vertexPositions.size()), modelParameters.m_Model);

        // Material
        if (modelParameters.m_AssimpScene->HasMaterials())
        {
            const auto assimpMaterial = modelParameters.m_AssimpScene->mMaterials[assimpMesh->mMaterialIndex];   // Get aiMaterial for this specific mesh.
            
            // Convert it to our material and add it to the model.
            std::shared_ptr<Material> material = LoadMaterial(assimpMaterial, modelParameters, parentEntity); // Convert it and add it to the model.
            modelParameters.m_Model->AddMaterial(material, parentEntity->GetPointerShared());
        }

        /// Bones.
    }

    std::shared_ptr<Material> Importer_Model::LoadMaterial(aiMaterial* assimpMaterial, const ModelParameters& modelParameters, Entity* materialEntity)
    {
        if (!assimpMaterial)
        {
            AURORA_WARNING(LogLayer::Graphics, "One of the provided materials is null. LoadMaterial cannot be executed.");
            return nullptr;
        }

        std::shared_ptr<Material> material = std::make_shared<Material>(m_EngineContext);

        // Name
        aiString materialName;
        aiGetMaterialString(assimpMaterial, AI_MATKEY_NAME, &materialName);
        
        // Set a resource file path so it can be used by the resource cache.
        material->SetResourceFilePath(FileSystem::RemoveIllegalCharacters(FileSystem::GetDirectoryFromFilePath(modelParameters.m_FilePath) + std::string(materialName.C_Str())) + EXTENSION_MATERIAL);

        // Diffuse Color
        aiColor4D diffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
        aiGetMaterialColor(assimpMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);

        // Opacity
        aiColor4D opacity(1.0f, 1.0f, 1.0f, 1.0f);
        aiGetMaterialColor(assimpMaterial, AI_MATKEY_OPACITY, &opacity);

        material->SetAlbedoColor({ diffuseColor.r, diffuseColor.g, diffuseColor.b, opacity.r });

        const auto LoadMaterialTexture = [this, &modelParameters, &assimpMaterial, &material](MaterialSlot engineSlotType, const aiTextureType assimpTextureTypePBR, const aiTextureType assimpTextureTypeLegacy)
        {
            aiTextureType typeAssimp = assimpMaterial->GetTextureCount(assimpTextureTypePBR) > 0 ? assimpTextureTypePBR : aiTextureType_NONE;
            typeAssimp = assimpMaterial->GetTextureCount(assimpTextureTypeLegacy) > 0 ? assimpTextureTypeLegacy : typeAssimp;

            aiString texturePath;

            if (assimpMaterial->GetTextureCount(typeAssimp) > 0)
            {
                if (AI_SUCCESS == assimpMaterial->GetTexture(typeAssimp, 0, &texturePath))
                {
                    const std::string filePath = ValidateTexturePath(texturePath.data, modelParameters.m_FilePath);
                    if (FileSystem::IsSupportedImageFile(filePath))
                    {
                        modelParameters.m_Model->AddTexture(material, engineSlotType, filePath);
                        //material->m_Textures[engineSlotType] = m_EngineContext->GetSubsystem<ResourceCache>()->Load<DX11_Texture>(deducedPath);
                        // m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(deducedPath, material->m_Textures[engineSlotType]);

                        if (typeAssimp == aiTextureType_BASE_COLOR || typeAssimp == aiTextureType_DIFFUSE)
                        {
                            // material->SetBaseColor({ 0.0f, 0.0f, 0.0f, 0.0f });
                        }

                        /// Normal/Height Map Stuff.

                        return; // Crucial, else we will revert to binding a default texture.
                    }
                }
            }

            // AURORA_WARNING(LogLayer::Graphics, "Could not find texture of appropriate type. Binding default texture...");
            // material->m_Textures[engineSlotType] = std::make_shared<AuroraResource>(m_EngineContext, Aurora::ResourceType::ResourceType_Image)->m_Texture;
            // m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(m_EngineContext->GetSubsystem<Renderer>()->m_DefaultWhiteTexture->m_FilePath, material->m_Textures[engineSlotType]);
            // material->m_Textures[engineSlotType] = m_EngineContext->GetSubsystem<Renderer>()->m_DefaultWhiteTexture;
        };

        // Engine Texture, Assimp PBR Texture, Assimp Legacy Texture (Fallback)
        LoadMaterialTexture(MaterialSlot::MaterialSlot_Albedo, aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE);
        LoadMaterialTexture(MaterialSlot::MaterialSlot_Roughness, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_SHININESS); // Use specular as fallback.
        LoadMaterialTexture(MaterialSlot::MaterialSlot_Metallic, aiTextureType_METALNESS, aiTextureType_AMBIENT); // Use ambient as fallback.
        LoadMaterialTexture(MaterialSlot::MaterialSlot_Normal, aiTextureType_NORMAL_CAMERA, aiTextureType_NORMALS);
        LoadMaterialTexture(MaterialSlot::MaterialSlot_Occlusion, aiTextureType_AMBIENT_OCCLUSION, aiTextureType_AMBIENT); // Use ambient as fallback.

        return material;
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
            AURORA_INFO(LogLayer::Graphics, "Model Texture Found: %s.", fullTexturePath.c_str());
            return fullTexturePath;
        }

        // 2) Check the same texture path as previously but this time with different file extensions (jpg, png and so on).
        fullTexturePath = TextureTryMultipleExtensions(fullTexturePath);
        if (FileSystem::Exists(fullTexturePath))
        {
            AURORA_INFO(LogLayer::Graphics, "Model Texture Found: %s.", fullTexturePath.c_str());
            return fullTexturePath;
        }

        // At this point, we know that the provided path is wrong, but we can make a few guesses. The most common mistake is that the arist provided a path relative to his/her computer.
        // 3) Check if the texture is in the same folder as the model.
        fullTexturePath = modelDirectory + FileSystem::GetFileNameFromFilePath(fullTexturePath);
        if (FileSystem::Exists(fullTexturePath))
        {
            AURORA_INFO(LogLayer::Graphics, "Model Texture Found: %s.", fullTexturePath.c_str());
            return fullTexturePath;
        }

        // 4) Check the same texture path as previously but with different file extensions (jpg, png and so on).
        fullTexturePath = TextureTryMultipleExtensions(fullTexturePath);
        if (FileSystem::Exists(fullTexturePath))
        {
            AURORA_INFO(LogLayer::Graphics, "Model Texture Found: %s.", fullTexturePath.c_str());
            return fullTexturePath;
        }

        // Give up, no valid texture path was found.
        AURORA_WARNING(LogLayer::Graphics, "Model Texture could not be found: %s.", fullTexturePath.c_str());
        return "";
    }
}