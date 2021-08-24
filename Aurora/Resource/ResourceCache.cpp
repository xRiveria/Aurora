#include "Aurora.h"
#include "ResourceCache.h"
#include "FileSystem.h"
#include "../Scene/World.h"
#include "../Renderer/Model.h"
#include "Importers/Importer_Model.h"
#include "Importers/Importer_Image.h"
#include "../Audio/AudioClip.h"
#include "../Scene/Components/Renderable.h"

namespace Aurora
{
    ResourceCache::ResourceCache(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        m_Importer_Model = std::make_shared<Importer_Model>(m_EngineContext);
        m_Importer_Image = std::make_shared<Importer_Image>(m_EngineContext);
    }

    ResourceCache::~ResourceCache()
    {
    }

    bool ResourceCache::Initialize()
    {
        return true;
    }

    bool ResourceCache::IsResourceCached(const std::string& resourceName, ResourceType resourceType)
    {
        AURORA_ASSERT(!resourceName.empty());

        for (std::shared_ptr<AuroraResource>& cachedResource : m_CachedResources)
        {
            if (resourceName == cachedResource->GetResourceName())
            {
                return true;
            }
        }
        
        return false;
    }

    std::shared_ptr<AuroraResource>& ResourceCache::GetResourceByName(const std::string& resourceName, ResourceType resourceType)
    {
        for (std::shared_ptr<AuroraResource>& cachedResource : m_CachedResources)
        {
            if (resourceName == cachedResource->GetResourceName())
            {
                return cachedResource;
            }
        }

        AURORA_INFO(LogLayer::Engine, "Empty Resource not found!");
        static std::shared_ptr<AuroraResource> emptyResource;
        return emptyResource;
    }

    std::vector<std::shared_ptr<AuroraResource>> ResourceCache::GetResourcesByType(ResourceType resourceType /*= ResourceType::Unknown */)
    {
        std::vector<std::shared_ptr<AuroraResource>> resources;

        for (std::shared_ptr<AuroraResource>& cachedResource : m_CachedResources)
        {
            if (cachedResource->GetResourceType() == resourceType || resourceType == ResourceType::ResourceType_Empty) // Allows us to pass in Empty and instead retrieve the total resource count.
            {
                resources.emplace_back(cachedResource);
            }
        }

        return resources;
    }

    uint64_t ResourceCache::GetMemoryUsageCPU(ResourceType resourceType)
    {
        uint64_t size = 0;

        for (std::shared_ptr<AuroraResource>& resource : m_CachedResources)
        {
            if (resource->GetResourceType() == resourceType || resourceType == ResourceType::ResourceType_Empty)
            {
                if (AuroraObject* auroraObject = dynamic_cast<AuroraResource*>(resource.get()))
                {
                    size += auroraObject->GetObjectSizeCPU();
                }
            }
        }

        return size;
    }

    uint64_t ResourceCache::GetMemoryUsageGPU(ResourceType resourceType)
    {
        uint64_t size = 0;

        for (std::shared_ptr<AuroraResource>& resource : m_CachedResources)
        {
            if (resource->GetResourceType() == resourceType || resourceType == ResourceType::ResourceType_Empty)
            {
                if (AuroraObject* auroraObject = dynamic_cast<AuroraResource*>(resource.get()))
                {
                    size += auroraObject->GetObjectSizeGPU();
;                }
            }
        }

        return size;
    }

    uint32_t ResourceCache::GetResourceCount(ResourceType resourceType)
    {
        return static_cast<uint32_t>(GetResourcesByType(resourceType).size());
    }

    void ResourceCache::ClearCache()
    {
        uint32_t resourceCount = static_cast<uint32_t>(m_CachedResources.size());

        m_CachedResources.clear();

        AURORA_INFO(LogLayer::Engine, "%u resources have been cleared from the Resource Cache.", resourceCount);
    }

    void ResourceCache::SaveResourcesToFiles()
    {
        /// Progress tracker.

        std::unique_ptr<FileSerializer> fileSerializer = std::make_unique<FileSerializer>(m_EngineContext);
        if (fileSerializer->BeginSerialization("ResourceCache"))
        {
            // Save resource count.
            const uint32_t resourceCount = GetResourceCount();
            fileSerializer->AddProperty("Resource_Count", resourceCount);

            uint32_t i = 0;

            // Save all currently used resources to disk.
            for (std::shared_ptr<AuroraResource>& resource : m_CachedResources)
            {
                if (!resource->HasFilePathNative())
                {
                    continue;
                }

                // Save file.
                std::string keyName = "Resource_" + std::to_string(i);

                fileSerializer->AddMapKey(keyName);
                fileSerializer->AddProperty("Resource_Path", resource->GetResourceFilePathNative());
                fileSerializer->AddProperty("Resource_Type", static_cast<uint32_t>(resource->GetResourceType()));
                fileSerializer->EndMapKey();

                // Save the state of the individual resource to ensure serialization/deserialization.
                resource->SaveToFile(resource->GetResourceFilePathNative());

                i++;
            }
        }
   
        fileSerializer->EndSerialization(FileSystem::GetDirectoryFromFilePath(m_EngineContext->GetSubsystem<World>()->GetWorldFilePath()) + m_EngineContext->GetSubsystem<World>()->GetWorldName() + "_Resources.dat");
    }

    void ResourceCache::LoadResourcesFromFiles()
    {
        std::unique_ptr<FileSerializer> fileSerializer = std::make_unique<FileSerializer>(m_EngineContext);
        uint32_t resourceCount = 0;
        uint32_t resourceType = 0;
        std::string filePath;

        if (fileSerializer->LoadFromFile(FileSystem::GetDirectoryFromFilePath(m_EngineContext->GetSubsystem<World>()->GetWorldFilePath()) + m_EngineContext->GetSubsystem<World>()->GetWorldName() + "_Resources.dat"))
        {
            if (fileSerializer->ValidateFileType("ResourceCache"))
            {
                // Load resource count.
                fileSerializer->GetProperty("Resource_Count", &resourceCount);

                for (uint32_t i = 0; i < resourceCount; i++)
                {
                    // Create resource key.
                    std::string keyName = "Resource_" + std::to_string(i);
                    // Get resource type.
                    fileSerializer->GetPropertyFromSubNode(keyName, "Resource_Type", &resourceType);
                    // Get resource path.
                    fileSerializer->GetPropertyFromSubNode(keyName, "Resource_Path", &filePath);
                   
                    const ResourceType resourceIdentified = static_cast<ResourceType>(resourceType);

                    switch (resourceIdentified)
                    {
                        case ResourceType::ResourceType_Material:
                            Load<Material>(filePath);
                            break;

                        case ResourceType::ResourceType_Model:
                            Load<Model>(filePath);
                            break;

                        case ResourceType::ResourceType_Image:
                            break;

                        case ResourceType::ResourceType_Audio:
                            Load<AudioClip>(filePath);
                            break;
                    }
                }
            }
        }
    }
}