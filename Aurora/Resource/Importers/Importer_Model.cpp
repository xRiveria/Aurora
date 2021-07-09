#include "Aurora.h"
#include "Importer_Model.h"
#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Material.h"
#include "../Scene/World.h"

#pragma warning(push, 0)
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader/tiny_obj_loader.h"
#pragma warning(pop)

namespace Aurora
{
    // Transform the data from OBJ space to engine space.
    static const bool g_TransformToLeftHandedCoordinates = true;

    Importer_Model::Importer_Model(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        
    }

    struct MemoryBuffer : std::streambuf
    {
        MemoryBuffer(char* begin, char* end)
        {
            this->setg(begin, begin, end); // Sets the value for the pointers that define the boundaries of the buffered portion of the controlled input sequence (eback, egptr) as well as the get pointer itself (gptr).
        }
    };

    // Custom material file reader.
    class MaterialFileReader : public tinyobj::MaterialReader
    {
    public:
        explicit MaterialFileReader(const std::string& mtlDirectory) : m_MTL_Directory(mtlDirectory) {}
        virtual ~MaterialFileReader() {}

        virtual bool operator()(const std::string& filePath, std::vector<tinyobj::material_t>* materials, std::map<std::string, int>* materialMap, std::string* error)
        {
            std::string materialPath;

            if (!m_MTL_Directory.empty())
            {
                materialPath = std::string(m_MTL_Directory) + filePath;
            }
            else
            {
                materialPath = filePath;
            }

            std::vector<uint8_t> materialFileData;
            if (!FileSystem::PushFileDataToBuffer(materialPath, materialFileData))
            {
                std::stringstream stringStream;
                stringStream << "Warning: Material File at [" << materialPath.c_str() << "] could not be found.\n";
                if (error)
                {
                    (*error) += stringStream.str();
                }

                return false;
            }

            MemoryBuffer streamBuffer((char*)materialFileData.data(), (char*)materialFileData.data() + materialFileData.size());
            std::istream materialIStream(&streamBuffer);

            std::string warning;
            std::string _error;
            tinyobj::LoadMtl(materialMap, materials, &materialIStream, &warning);

            if (!warning.empty())
            {
                if (error)
                {
                    (*error) += warning;
                }
            }

            return true;
        }

    private:
        std::string m_MTL_Directory;
    };

    void Importer_Model::LoadModel_OBJ(const std::string& filePath)
    {
        World* world = m_EngineContext->GetSubsystem<World>();

        std::string modelDirectory = FileSystem::GetDirectoryFromFilePath(filePath);
        std::string modelName = FileSystem::GetNameFromFilePath(filePath);

        tinyobj::attrib_t objectAttributes;
        std::vector<tinyobj::shape_t> objectShapes;
        std::vector<tinyobj::material_t> objectMaterials;
        std::string objectErrors;

        std::vector<uint8_t> fileData;
        bool success = FileSystem::PushFileDataToBuffer(filePath, fileData);

        if (success)
        {
            MemoryBuffer streamBuffer((char*)fileData.data(), (char*)fileData.data() + fileData.size());
            std::istream in(&streamBuffer);
            MaterialFileReader materialFileReader(modelDirectory);
            success = tinyobj::LoadObj(&objectAttributes, &objectShapes, &objectMaterials, &objectErrors, &in, &materialFileReader, true);
        }
        else
        {
            objectErrors = "Failed to read file: " + filePath;
        }

        if (!objectErrors.empty())
        {
            AURORA_ERROR(objectErrors.c_str());
        }

        if (success)
        {
            // Load material library.
            std::vector<Entity> materialLibrary = {}; // Heap.

            for (tinyobj::material_t& objectMaterial : objectMaterials)
            {
                /// For now, each material component will be its own entity.
                std::shared_ptr<Entity> newEntity = world->EntityCreate();
                newEntity->SetName(objectMaterial.name + "_SubMaterial");
                Material materialComponent = *newEntity.get()->AddComponent<Material>();

                materialComponent.m_BaseColor = XMFLOAT4(objectMaterial.diffuse[0], objectMaterial.diffuse[1], objectMaterial.diffuse[2], 1);
                materialComponent.m_Textures[Texture_Slot::Base_Color_Map].m_FilePath = objectMaterial.diffuse_texname;
                materialComponent.m_Textures[Texture_Slot::Displacement_Map].m_FilePath = objectMaterial.displacement_texname;

                materialComponent.m_EmissiveColor.x = objectMaterial.emission[0];
                materialComponent.m_EmissiveColor.y = objectMaterial.emission[1];
                materialComponent.m_EmissiveColor.z = objectMaterial.emission[2];
                materialComponent.m_EmissiveColor.w = std::max(objectMaterial.emission[0], std::max(objectMaterial.emission[1], objectMaterial.emission[2])); // Strength.

                materialComponent.m_Metalness = objectMaterial.metallic;
                materialComponent.m_Roughness = objectMaterial.roughness;
                
                materialComponent.m_Textures[Texture_Slot::Normal_Map].m_FilePath = objectMaterial.normal_texname;
                materialComponent.m_Textures[Texture_Slot::Surface_Map].m_FilePath = objectMaterial.specular_texname;
                
                if (materialComponent.m_Textures[Texture_Slot::Normal_Map].m_FilePath.empty())
                {
                    materialComponent.m_Textures[Texture_Slot::Normal_Map].m_FilePath = objectMaterial.bump_texname;
                }
                if (materialComponent.m_Textures[Texture_Slot::Surface_Map].m_FilePath.empty())
                {
                    materialComponent.m_Textures[Texture_Slot::Surface_Map].m_FilePath = objectMaterial.specular_highlight_texname;
                }

                for (auto& texture : materialComponent.m_Textures)
                {
                    if (!texture.m_FilePath.empty())
                    {
                        texture.m_FilePath = modelDirectory + texture.m_FilePath;
                    }
                }

                materialComponent.CreateRenderData();

                materialLibrary.push_back(*newEntity.get()); // For subset indexing.
            }

            if (materialLibrary.empty())
            {
                // Create default material if nothing was found.
                std::shared_ptr<Entity> newEntity = world->EntityCreate();
                newEntity->SetName("OBJImporter_DefaultMaterial");
                Material materialComponent = *newEntity.get()->AddComponent<Material>(); // Subset Indexing

                materialLibrary.push_back(*newEntity.get());
            }

            // Load objects and meshes.
            for (tinyobj::shape_t& shape : objectShapes)
            {
                /// We will turn them into child entities in the future if they belong to the same model.
                // Each shape, or submesh is it's own entity.
                std::shared_ptr<Entity> newEntity = world->EntityCreate();
                newEntity->SetName(shape.name + "_SubMesh");
                Mesh meshComponent = *newEntity.get()->AddComponent<Mesh>();

                std::unordered_map<size_t, uint32_t> uniqueVertices = {};
                std::unordered_map<int, int> registeredMaterialIndices = {};

                for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
                {
                    tinyobj::index_t reorderedIndices[] = {
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

                    for (tinyobj::index_t& index : reorderedIndices)
                    {
                        XMFLOAT3 position = XMFLOAT3(
                            objectAttributes.vertices[index.vertex_index * 3 + 0],
                            objectAttributes.vertices[index.vertex_index * 3 + 1],
                            objectAttributes.vertices[index.vertex_index * 3 + 2]
                        );
                        
                        XMFLOAT3 normal = XMFLOAT3(0, 0, 0);
                        if (!objectAttributes.normals.empty())
                        {
                            normal = XMFLOAT3(
                                objectAttributes.normals[index.normal_index * 3 + 0],
                                objectAttributes.normals[index.normal_index * 3 + 1],
                                objectAttributes.normals[index.normal_index * 3 + 2]
                            );
                        }

                        XMFLOAT2 texCoord = XMFLOAT2(0, 0);
                        // The OBJ format assumes a coordinate system where a vertical coordinate of 0 means the bottom of the image. However, for us, 0 means the top of the image. Hence, we flip the vertical component of the coordinates passed in.
                        if (index.texcoord_index >= 0 && !objectAttributes.texcoords.empty())
                        {
                            texCoord = XMFLOAT2(
                                objectAttributes.texcoords[index.texcoord_index * 2 + 0],
                                1 - objectAttributes.texcoords[index.texcoord_index * 2 + 1]
                            );
                        }

                        int materialIndex = std::max(0, shape.mesh.material_ids[i / 3]); // This indexes the material library.
                        if (registeredMaterialIndices.count(materialIndex) == 0)
                        {
                            registeredMaterialIndices[materialIndex] = (int)meshComponent.m_Subsets.size();
                            meshComponent.m_Subsets.push_back(Mesh::MeshSubset());
                            meshComponent.m_Subsets.back().m_Material_Entity = &materialLibrary[materialIndex];
                            meshComponent.m_Subsets.back().m_Index_Offset = (uint32_t)meshComponent.m_Indices.size();
                        }

                        if (g_TransformToLeftHandedCoordinates)
                        {
                            position.z *= -1;
                            normal.z *= -1;
                        }

                        // Eliminate duplicate vertices by means f hashing.
                        size_t vertexHash = 0;
                        Utilities::Hash_Combine(vertexHash, index.vertex_index);
                        Utilities::Hash_Combine(vertexHash, index.normal_index);
                        Utilities::Hash_Combine(vertexHash, index.texcoord_index);
                        Utilities::Hash_Combine(vertexHash, materialIndex);

                        if (uniqueVertices.count(vertexHash) == 0)  // Searches the container for element key and returns any elements found.
                        {
                            uniqueVertices[vertexHash] = (uint32_t)meshComponent.m_Vertex_Positions.size(); // Size is incremented here per looped index.
                            meshComponent.m_Vertex_Positions.push_back(position);
                            meshComponent.m_Vertex_Normals.push_back(normal);
                            meshComponent.m_Vertex_UVSet_0.push_back(texCoord);
                        }

                        meshComponent.m_Indices.push_back(uniqueVertices[vertexHash]);
                        meshComponent.m_Subsets.back().m_Index_Count++;
                    }
                }

                meshComponent.CreateRenderData();
            }

            /// Scene Update.
        }
        else
        {
            AURORA_ERROR("Failed to load Model at %s.", filePath.c_str());
            return;
        }
    }
}