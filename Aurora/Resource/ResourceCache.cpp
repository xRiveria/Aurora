#include "Aurora.h"
#include "ResourceCache.h"
#include "FileSystem.h"
#include "../Scene/World.h"
#include "Importers/Importer_Model.h"
#include "Importers/Importer_Image.h"

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

    std::vector<std::shared_ptr<AuroraResource>> ResourceCache::GetResourcesByType(ResourceType resourceType)
    {
        std::vector<std::shared_ptr<AuroraResource>> resources;

        for (std::shared_ptr<AuroraResource>& cachedResource : m_CachedResources)
        {
            if (cachedResource->GetResourceType() == resourceType)
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

        // Create resource list file.
        std::string filePath = m_EngineContext->GetSubsystem<Settings>()->GetProjectDirectoryAbsolute() + m_EngineContext->GetSubsystem<World>()->GetWorldName() + "_Resources.dat";       
    }

    void ResourceCache::LoadResourcesFromFiles()
    {

    }

    // =========================================================================================

    bool ResourceCache::LoadModel(const std::string& filePath, std::shared_ptr<AuroraResource> resource, bool cacheResource /*= true*/)
    {
        if (FileSystem::IsSupportedModelFile(filePath))
        {
            if (cacheResource)
            {
                m_CachedResources.push_back(resource);
            }
            return m_Importer_Model->LoadModel(filePath, resource.get());
        }

        AURORA_ERROR(LogLayer::Engine, "Requested model file is not supported: %s.", filePath.c_str());
        return false;
    }
}