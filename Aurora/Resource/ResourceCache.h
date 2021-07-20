#pragma once
#include "ResourceUtilities.h"
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
        DefaultObjectType_Capsule,
        DefaultObjectType_Cylinder,
        DefaultObjectType_Plane
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

        /* Load a Resource
            
            - Name: File name of the resource.
            - Flags: Specify flags that modify behavior (optional).
            - File Data: Pointer to file data, if file was loaded manually (optional).
            - File Size: Size of file data, if file was loaded manually (optional).
        */

        std::shared_ptr<AuroraResource> LoadTexture(const std::string& filePath, const std::string& fileName = "", uint32_t loadFlags = 0);
        std::shared_ptr<Entity> LoadModel(const std::string& filePath, const std::string& fileName = "");
        void MapDefaultObjects();

    public:
        /// Map directories for each path type and expose in editor.
        std::unordered_map<std::string, std::shared_ptr<AuroraResource>> m_Resources;  // We temporarily store everything.
        
        std::unordered_map<DefaultObjectType, std::string> m_DefaultObjects; // Maps an object with a file path.

        std::shared_ptr<Importer_Model> m_Importer_Model = nullptr;
        std::shared_ptr<Importer_Image> m_Importer_Image = nullptr;
    };
}