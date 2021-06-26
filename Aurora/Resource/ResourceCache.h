#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>
#include "ISubsystem.h"
#include "ResourceUtilities.h"

namespace Aurora
{
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
        std::shared_ptr<AuroraResource> Load(const std::string& fileName, const std::string& filePath, uint32_t flags = 0);

    private:
        std::unordered_map<std::string, std::shared_ptr<AuroraResource>> m_Resources;  // We temporarily store everything.
    };
}