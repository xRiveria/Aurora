#pragma once
#include "AuroraResource.h"
#include "ISubsystem.h"

/*
    We have 2 primary types of resource formats: native and global formats. Native ones are essentially resources created by the engine, such as scene files, materials etc.
    Global formats are your typical FBX, JPG or PNG formats.
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

        bool Initialize() override;

        bool LoadModel(const std::string& filePath, std::shared_ptr<AuroraResource> resource, bool cacheResource = true);
        
        // =========================================================================================================

        // Get by Name
        std::shared_ptr<AuroraResource>& GetResourceByName(const std::string& resourceName, ResourceType resourceType);

        template<typename T>
        constexpr std::shared_ptr<T> GetResourceByName(const std::string& resourceName)
        {
            return std::static_pointer_cast<T>(GetResourceByName(resourceName, AuroraResource::TypeToEnum<T>()));
        }

        // Get by Type.
        std::vector<std::shared_ptr<AuroraResource>> GetResourcesByType(ResourceType resourceType = ResourceType::ResourceType_Empty);

        // Get by Path.
        template <typename T>
        std::shared_ptr<T> GetResourceByPath(const std::string& filePath)
        {
            for (std::shared_ptr<AuroraResource>& resource : m_CachedResources)
            {
                if (filePath == resource->GetResourceFilePathNative())
                {
                    return std::static_pointer_cast<T>(resource);
                }
            }

            return nullptr;
        }

        bool IsResourceCached(const std::string& resourceName, ResourceType resourceType);
        
        // Caches resource, or replaces with existing cached resource.
        template <typename T>
        [[nodiscard]] std::shared_ptr<T> CacheResource(const std::shared_ptr<T>& resource)
        {
            // Validate resource.
            if (!resource)
            {
                return nullptr;
            }

            // Validate resource file path.
            if (!resource->HasFilePathNative() && !FileSystem::IsDirectory(resource->GetResourceFilePathNative()))
            {
                AURORA_ERROR(LogLayer::Engine, "A resource must have a valid file path in order to be cached.");
                return nullptr;
            }

            // Validate resource file path.
            //if (!FileSystem::IsEngineFile(resource->GetResourceFilePathNative()))
            //{
            //    AURORA_ERROR(LogLayer::Engine, "A resource must have a native file format in order to be cached. The provided format was %s.", FileSystem::GetExtensionFromFilePath(resource->GetResourceFilePathNative()).c_str());
            //    return nullptr;
            //}

            // Ensure that this resource is not already cached.
            if (IsResourceCached(resource->GetResourceName(), resource->GetResourceType()))
            {
                return GetResourceByName<T>(resource->GetResourceName());
            }

            // In order to guarentee deserialization, we save it now.
            resource->SaveToFile(resource->GetResourceFilePathNative());

            // Cache it.
            return std::static_pointer_cast<T>(m_CachedResources.emplace_back(resource));
        }

        // Loads a resource and adds it to the resource cache.
        template<typename T>
        std::shared_ptr<T> Load(const std::string& filePath)
        {
            if (!FileSystem::Exists(filePath))
            {
                AURORA_ERROR(LogLayer::Engine, "Requested resource at \"%s\" does not exist.", filePath.c_str());
                return nullptr;
            }

            // Check if the resource is already loaded. If so, we will retrieve it.
            const std::string resourceName = FileSystem::GetFileNameWithoutExtensionFromFilePath(filePath);
            if (IsResourceCached(resourceName, AuroraResource::TypeToEnum<T>()))
            {
                return GetResourceByName<T>(resourceName);
            }

            // Create a new resource and set its file path.
            auto typedResource = std::make_shared<T>(m_EngineContext);
            typedResource->SetResourceFilePath(filePath);

            // Load
            if (!typedResource || !typedResource->LoadFromFile(filePath))
            {
                AURORA_ERROR(LogLayer::Engine, "Failed to load resource at path \"%s\".", filePath.c_str());
                return nullptr;
            }

            // Return cached reference which is guaranteed to remain around after deserialization.
            return CacheResource<T>(typedResource);
        }

        // Misc - Memory
        uint64_t GetMemoryUsageCPU(ResourceType resourceType = ResourceType::ResourceType_Empty);
        uint64_t GetMemoryUsageGPU(ResourceType resourceType = ResourceType::ResourceType_Empty);
        // Returns all resource count of a particular types.
        uint32_t GetResourceCount(ResourceType resourceType = ResourceType::ResourceType_Empty);
        void ClearCache();

        // Importers
        Importer_Model* GetModelImporter() const { return m_Importer_Model.get(); }
        Importer_Image* GetImageImporter() const { return m_Importer_Image.get(); }

    public:
        // Cache
        std::vector<std::shared_ptr<AuroraResource>> m_CachedResources;

    private:
        void SaveResourcesToFiles();
        void LoadResourcesFromFiles();

    private:
        // Importers
        std::shared_ptr<Importer_Model> m_Importer_Model = nullptr;
        std::shared_ptr<Importer_Image> m_Importer_Image = nullptr;
    };
}