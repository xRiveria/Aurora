#include "Aurora.h"
#include "Settings.h"

namespace Aurora
{
    Settings::Settings(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        const std::string dataDirectory = "..\\Resources\\";

        // Standard Resource Directories
        AddResourceDirectory(ResourceDirectory::Fonts, dataDirectory + "Fonts");
        AddResourceDirectory(ResourceDirectory::Icons, dataDirectory + "Icons");
        AddResourceDirectory(ResourceDirectory::Textures, dataDirectory + "Textures");
        AddResourceDirectory(ResourceDirectory::Shaders, dataDirectory + "Shaders");
        AddResourceDirectory(ResourceDirectory::Cubemaps, dataDirectory + "Cubemaps");
        AddResourceDirectory(ResourceDirectory::Models, dataDirectory + "Models");
        AddResourceDirectory(ResourceDirectory::Scenes, dataDirectory + "Scenes");
    
        // Create Project Directory
        SetProjectDirectory("Project/");

        // Register External Libraries - We usually do this in their initialization loops. However, certain ones may not have an initialization procedure.
        RegisterExternalLibrary("yaml-cpp", "0.7.0", "https://github.com/jbeder/yaml-cpp");
        RegisterExternalLibrary("stb_image", "2.27", "https://github.com/nothings/stb");
        RegisterExternalLibrary("DirectXCompiler", "1.6.2106", "https://github.com/microsoft/DirectXShaderCompiler");
    }

    Settings::~Settings()
    {

    }

    void Settings::RegisterExternalLibrary(const std::string& libraryName, const std::string& libraryVersion, const std::string& libraryURL)
    {
        m_ExternalLibraries.emplace_back(libraryName, libraryVersion, libraryURL);
    }

    void Settings::AddResourceDirectory(ResourceDirectory resourceType, const std::string& directoryPath)
    {
        m_ResourceDirectories[resourceType] = directoryPath;
    }

    void Settings::SetResourceDirectory(ResourceDirectory resourceDirectory, const std::string& directoryPath)
    {
        for (const std::pair<ResourceDirectory, std::string>& directory : m_ResourceDirectories)
        {
            if (directory.first == resourceDirectory)
            {
                m_ResourceDirectories[directory.first] = directoryPath;
                return;
            }
        }

        AURORA_WARNING("Resource type does not exist in directory mapping: \"%s\".", directoryPath);
    }

    std::string Settings::GetResourceDirectory(ResourceDirectory resourceDirectory)
    {
        for (const std::pair<ResourceDirectory, std::string>& directory : m_ResourceDirectories)
        {
            if (directory.first == resourceDirectory)
            {
                return directory.second;
            }
        }

        return "Null Path";
    }

    void Settings::SetProjectDirectory(const std::string& directoryPath)
    {
        if (!FileSystem::Exists(directoryPath))
        {
            FileSystem::CreateDirectory_(directoryPath);
        }

        m_ProjectDirectory = directoryPath;
    }

    std::string Settings::GetProjectDirectoryAbsolute() const
    {
        return FileSystem::GetWorkingDirectory() + "/" + m_ProjectDirectory;
    }
}