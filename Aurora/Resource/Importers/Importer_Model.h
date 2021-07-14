#pragma once
#include <string>

struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiScene;

namespace Aurora
{
    class EngineContext;
    class Entity;
    class World;
    class Material;

    struct ModelParameters
    {
        uint32_t m_TriangleLimit;
        uint32_t m_VertexLimit;
        float m_MaxNormalSmoothingAngle;
        float m_MaxTangentSmoothingAngle;

        std::string m_FilePath;
        std::string m_Name;
        bool m_HasAnimations;

        const aiScene* m_AssimpScene = nullptr;
    };

    class Importer_Model
    {
    public:
        Importer_Model(EngineContext* engineContext);
        ~Importer_Model() = default;

        void Load(const std::string& filePath);

    private:
        bool ImporterModel_General(const std::string& filePath);
        // void ImportModel_OBJ(const std::string& filePath);

        // Parsing
        void ParseNode(const aiNode* assimpNode, const ModelParameters& modelParameters, Entity* parentEntity = nullptr, Entity* newEntity = nullptr);
        void ParseNodeMeshes(const aiNode* assimpNode, Entity* newEntity, const ModelParameters& modelParameters);
        // void ParseAnimations(const ModelParameters& modelParameters);

        // Loading
        void LoadMesh(aiMesh* assimpMesh, Entity* parentEntity, const ModelParameters& modelParameters);
        void LoadMaterial(aiMaterial* assimpMaterial, const ModelParameters& modelParameters, Entity* materialEntity);

        // Helpers
        std::string TextureTryMultipleExtensions(const std::string& filePath);
        std::string ValidateTexturePath(std::string originalTexturePath, const std::string& modelPath);

    private:
        EngineContext* m_EngineContext;
        World* m_WorldContext;
    };
}