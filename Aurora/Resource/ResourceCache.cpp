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

        MapDefaultObjects();
    }

    ResourceCache::~ResourceCache()
    {

    }

    std::shared_ptr<Entity> ResourceCache::LoadModel(const std::string& filePath, const std::string& fileName)
    {
        if (FileSystem::IsSuppportedModelFile(filePath))
        {
            return m_Importer_Model->Load(filePath, fileName);
        }

        AURORA_ERROR("Requested model file not supported: %s.", filePath.c_str());
        return nullptr;
    }

    void ResourceCache::MapDefaultObjects()
    {
        m_DefaultObjects[DefaultObjectType::DefaultObjectType_Cube] = "../Resources/Models/Default/Cube/scene.gltf";
        m_DefaultObjects[DefaultObjectType::DefaultObjectType_Sphere] = "../Resources/Models/Default/Sphere/globe-sphere.obj";
    }

    std::shared_ptr<AuroraResource> ResourceCache::LoadTexture(const std::string& filePath, const std::string& fileName, uint32_t loadFlags)
    {
        if (FileSystem::IsSupportedImageFile(filePath))
        {
            return m_Importer_Image->LoadTexture(filePath, fileName, loadFlags);
        }

        AURORA_ERROR("Requested image file is not supported: %s.", filePath.c_str());
        return nullptr;
    }
}