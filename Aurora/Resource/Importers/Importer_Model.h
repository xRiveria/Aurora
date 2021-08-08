#pragma once
#include <string>
#include "../Graphics/RHI_Vertex.h"
#include "../Graphics/DX11_Refactored/DX11_IndexBuffer.h"

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

        std::shared_ptr<Entity> Load(const std::string& filePath, const std::string& fileName = "");

    private:
        // Parsing
        void ParseNode(const aiNode* assimpNode, const ModelParameters& modelParameters, Entity* parentEntity = nullptr, Entity* newEntity = nullptr);
        void ParseNodeMeshes(const aiNode* assimpNode, Entity* newEntity, const ModelParameters& modelParameters);

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

class MeshDerp
{
public:
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT3 tangent;
        XMFLOAT3 bitangent;
        XMFLOAT2 texcoord;
    };
    static_assert(sizeof(Vertex) == 14 * sizeof(float));
    static const int NumAttributes = 5;

    struct Face
    {
        uint32_t v1, v2, v3;
    };
    static_assert(sizeof(Face) == 3 * sizeof(uint32_t));

    static std::shared_ptr<MeshDerp> fromFile(const std::string& filename);
    static std::shared_ptr<MeshDerp> fromString(const std::string& data);

    const std::vector<Vertex>& vertices() const { return m_vertices; }
    const std::vector<Face>& faces() const { return m_faces; }
    std::vector<Aurora::RHI_Vertex_Position_UV_Normal> m_Vertices;
    std::vector<uint32_t> m_Indices;

private:
    MeshDerp(const struct aiMesh* mesh);

    std::vector<Vertex> m_vertices;
    std::vector<Face> m_faces;
};