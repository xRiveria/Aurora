#pragma once
#include <string>

namespace Aurora
{
    enum class ResourceDirectory
    {
        Shaders,
        Scenes,
        Textures,
        Scripts,
        Icons,
        Fonts,
        Models,
        Cubemaps
    };

    inline const char* ToString(ResourceDirectory directoryType)
    {
        switch (directoryType)
        {
            case ResourceDirectory::Shaders:
                return "Shaders";

            case ResourceDirectory::Scenes:
                return "Scenes";

            case ResourceDirectory::Textures:
                return "Textures";

            case ResourceDirectory::Scripts:
                return "Scripts";

            case ResourceDirectory::Icons:
                return "Icons";

            case ResourceDirectory::Fonts:
                return "Fonts";

            case ResourceDirectory::Models:
                return "Models";

            case ResourceDirectory::Cubemaps:
                return "Cubemaps";
        }

        return "Unidentified Resource Directory";
    }

    struct ExternalLibrary
    {
        ExternalLibrary(const std::string& libraryName, const std::string& libraryVersion, const std::string& libraryURL)
        {
            this->m_Name = libraryName;
            this->m_Version = libraryVersion;
            this->m_URL = libraryURL;
        }

        std::string m_Name;
        std::string m_Version;
        std::string m_URL;
    };
}