#pragma once
#include "AuroraResource.h"
#include "ISubsystem.h"

/*
    - Resource Registry.
    - Total Vertice/Indices
*/

namespace Aurora
{
    enum class DefaultObjectType
    {
        DefaultObjectType_Cube,
        DefaultObjectType_Sphere,
        DefaultObjectType_Torus,
        DefaultObjectType_Cone,
        DefaultObjectType_Plane,
        DefaultObjectType_Cylinder,
        DefaultObjectType_PointLight
    };

    class AuroraResource;
    class Entity;
    class Importer_Model;
    class Importer_Image;

    class ResourceCache : public ISubsystem
    {
    public:
        ResourceCache(EngineContext* engineContext);
        ~ResourceCache();

        bool LoadTexture(const std::string& filePath, std::shared_ptr<AuroraResource> resource);
        // bool LoadModel(const std::string& filePath, AuroraResource& resource);
        
        // =========================================================================================================

        std::string GetResourceDirectory() const { return m_ResourceDirectory; }
        std::shared_ptr<AuroraResource> LoadTexture(const std::string& filePath, const std::string& fileName = "", uint32_t loadFlags = 0);
        std::shared_ptr<AuroraResource> LoadTextureHDR(const std::string& filePath, int channels = 4);
        std::shared_ptr<Entity> LoadModel(const std::string& filePath, const std::string& fileName = "");
        void MapDefaultObjects();

        auto& GetCachedResources() const { return m_Resources; }

    public:
        /// Map directories for each path type and expose in editor.

        std::vector<std::shared_ptr<AuroraResource>> m_Resources;
        std::unordered_map<DefaultObjectType, std::string> m_DefaultObjects; // Maps an object with a file path.

        std::shared_ptr<Importer_Model> m_Importer_Model = nullptr;
        std::shared_ptr<Importer_Image> m_Importer_Image = nullptr;
        std::string m_ResourceDirectory = "../Resources";
    };
}