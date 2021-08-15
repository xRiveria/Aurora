#include "Aurora.h"
#include "ResourceCache.h"
#include "FileSystem.h"
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
        CreateDefaultObjects();
        return true;
    }

    bool ResourceCache::LoadTexture(const std::string& filePath, std::shared_ptr<AuroraResource> resource, bool cacheResource /*= true*/)
    {
        if (FileSystem::IsSupportedImageFile(filePath))
        {
            if (cacheResource)
            {
                m_Resources.push_back(resource);
            }
            return m_Importer_Image->LoadTexture(filePath, resource.get());
        }

        AURORA_ERROR(LogLayer::Engine, "Requested image file is not supported: %s.", filePath.c_str());
        return false;
    }

    bool ResourceCache::LoadModel(const std::string& filePath, std::shared_ptr<AuroraResource> resource, bool cacheResource /*= true*/)
    {
        if (FileSystem::IsSupportedModelFile(filePath))
        {
            if (cacheResource)
            {
                m_Resources.push_back(resource);
            }
            return m_Importer_Model->LoadModel(filePath, resource.get());
        }

        AURORA_ERROR(LogLayer::Engine, "Requested model file is not supported: %s.", filePath.c_str());
        return false;
    }

    void ResourceCache::CreateDefaultObjects()
    {
        /*
        m_DefaultObjects[DefaultObjectType::DefaultObjectType_Cube] = std::make_shared<AuroraResource>();
        m_Importer_Model->LoadModel("../Resources/Models/Default/Cube.fbx", m_DefaultObjects[DefaultObjectType::DefaultObjectType_Cube].get());

        m_DefaultObjects[DefaultObjectType::DefaultObjectType_Sphere] = std::make_shared<AuroraResource>();
        m_Importer_Model->LoadModel("../Resources/Models/Default/Sphere.fbx", m_DefaultObjects[DefaultObjectType::DefaultObjectType_Sphere].get());

        m_DefaultObjects[DefaultObjectType::DefaultObjectType_Plane] = std::make_shared<AuroraResource>();
        m_Importer_Model->LoadModel("../Resources/Models/Default/Plane.fbx", m_DefaultObjects[DefaultObjectType::DefaultObjectType_Plane].get());

        m_DefaultObjects[DefaultObjectType::DefaultObjectType_Cylinder] = std::make_shared<AuroraResource>();
        m_Importer_Model->LoadModel("../Resources/Models/Default/Cylinder.fbx", m_DefaultObjects[DefaultObjectType::DefaultObjectType_Cylinder].get());

        m_DefaultObjects[DefaultObjectType::DefaultObjectType_Torus] = std::make_shared<AuroraResource>();
        m_Importer_Model->LoadModel("../Resources/Models/Default/Torus.fbx", m_DefaultObjects[DefaultObjectType::DefaultObjectType_Torus].get());

        m_DefaultObjects[DefaultObjectType::DefaultObjectType_Cone] = std::make_shared<AuroraResource>();
        m_Importer_Model->LoadModel("../Resources/Models/Default/Cone.fbx", m_DefaultObjects[DefaultObjectType::DefaultObjectType_Cone].get());
        */
    }

    /*
    std::shared_ptr<AuroraResource> ResourceCache::LoadTexture(const std::string& filePath, const std::string& fileName, uint32_t loadFlags)
    {
        if (FileSystem::IsSupportedImageFile(filePath))
        {
            return m_Importer_Image->LoadTexture(filePath, fileName, loadFlags);
        }

        AURORA_ERROR(LogLayer::Engine, "Requested image file is not supported: %s.", filePath.c_str());
        return nullptr;
    }

    std::shared_ptr<AuroraResource> ResourceCache::LoadTextureHDR(const std::string& filePath, int channels)
    {
        return m_Importer_Image->LoadHDRTexture(filePath, channels);
    }
    */
}