#pragma once
#include "EngineContext.h"
#include "../Core/ISubsystem.h"
#include "SettingsUtilities.h"
#include "../Math/Vector2.h"

namespace Aurora
{
    class Settings : public ISubsystem
    {
    public:
        Settings(EngineContext* engineContext);
        ~Settings();

        // Subsystem
        bool Initialize() override;

        // Name
        void SetApplicationName(const std::string& applicationName) { m_ApplicationName = applicationName; }
        std::string& GetApplicationName() { return m_ApplicationName; }

        // Register External Libraries
        void RegisterExternalLibrary(const std::string& libraryName, const std::string& libraryVersion, const std::string& libraryURL);
        const auto& GetThirdPartyLibraries() const { return m_ExternalLibraries; }

        // Resource Directories
        void AddResourceDirectory(ResourceDirectory resourceType, const std::string& directoryPath);
        void SetResourceDirectory(ResourceDirectory resourceDirectory, const std::string& directoryPath);
        std::string GetResourceDirectory(ResourceDirectory resourceDirectory);
        std::string GetResourceDirectory() const { return "Resources"; }
        std::unordered_map<ResourceDirectory, std::string>& GetResourceDirectories() { return m_ResourceDirectories; }

        // Project Directories
        void SetProjectDirectory(const std::string& directoryPath);
        std::string& GetProjectDirectory() { return m_ProjectDirectory; }
        std::string GetProjectDirectoryAbsolute() const;

    private:
        std::string m_ApplicationName = "Application";
        Math::Vector2 m_Resolution_Output = Math::Vector2::Zero;

        std::string m_ProjectDirectory;
        std::unordered_map<ResourceDirectory, std::string> m_ResourceDirectories;
        std::vector<ExternalLibrary> m_ExternalLibraries;
    };
}
