#include "Aurora.h"
#include "FileSystem.h"
#include <filesystem>
#include <regex>
#include <Windows.h>
#include <shellapi.h>
#include <fstream>

namespace Aurora
{
    std::string FileSystem::ConvertToUppercase(const std::string& lowerString)
    {
        const std::locale locale;
        std::string upperString;

        for (const char& character : lowerString)
        {
            upperString += std::toupper(character, locale);
        }

        return upperString;
    }

    std::wstring FileSystem::StringToWString(const std::string& string)
    {
        const auto sLength = static_cast<int>(string.length()) + 1;
        const auto length = MultiByteToWideChar(CP_ACP, 0, string.c_str(), sLength, nullptr, 0);
        const auto buffer = new wchar_t[length];
        MultiByteToWideChar(CP_ACP, 0, string.c_str(), sLength, buffer, length);
        std::wstring result(buffer);
        delete[] buffer;
        return result;
    }

    std::string FileSystem::WStringToString(const std::wstring& wString)
    {
        int num = WideCharToMultiByte(CP_UTF8, 0, wString.c_str(), -1, NULL, 0, NULL, NULL);
        std::string buffer;
        buffer.resize(size_t(num) - 1);
        WideCharToMultiByte(CP_UTF8, 0, wString.c_str(), -1, &buffer[0], num, NULL, NULL);

        return buffer;
    }

    bool FileSystem::CreateDirectory_(const std::string& directoryPath)
    {
        try
        {
            if (std::filesystem::create_directories(directoryPath))
            {
                return true;
            }
        }
        catch (std::filesystem::filesystem_error& error)
        {
            AURORA_WARNING(LogLayer::Engine, "%s, %s", error.what(), directoryPath.c_str());
        }

        return false;
    }

    bool FileSystem::Exists(const std::string& filePath)
    {
        try
        {
            if (std::filesystem::exists(filePath))
            {
                return true;
            }
        }
        catch (std::filesystem::filesystem_error& error)
        {
            AURORA_WARNING(LogLayer::Engine, "%s, %s", error.what(), filePath.c_str());
        }

        return false;
    }

    bool FileSystem::Delete(const std::string& filePath)
    {
        try
        {
            if (std::filesystem::exists(filePath) && std::filesystem::remove_all(filePath))
            {
                return true;
            }
        }
        catch (std::filesystem::filesystem_error& error)
        {
            AURORA_ERROR(LogLayer::Editor, "Failed to delete \"%s\". %s", filePath.c_str(), error.what());
        }

        return false;
    }

    bool FileSystem::IsDirectory(const std::string& filePath)
    {
        try
        {
            if (std::filesystem::exists(filePath) && std::filesystem::is_directory(filePath))
            {
                return true;
            }
        }
        catch (std::filesystem::filesystem_error& error)
        {
            AURORA_WARNING(LogLayer::Engine, "%s, %s", error.what(), filePath.c_str());
        }

        return false;
    }

    bool FileSystem::IsFile(const std::string& filePath)
    {
        if (filePath.empty())
        {
            return false;
        }

        try
        {
            if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
            {
                return true;
            }
        }
        catch (std::filesystem::filesystem_error& error)
        {
            AURORA_WARNING(LogLayer::Engine, "%s, %s", error.what(), filePath.c_str())
        }

        return false;
    }

    void FileSystem::OpenDirectoryWindow(const std::string& directory)
    {
        ShellExecute(nullptr, nullptr, StringToWString(directory).c_str(), nullptr, nullptr, SW_SHOW);
    }

    std::string FileSystem::GetDirectoryFromFilePath(const std::string& filePath)
    {
        const size_t lastIndex = filePath.find_last_of("\\/");

        if (lastIndex != std::string::npos)
        {
            return filePath.substr(0, lastIndex + 1); // To keep our /.
        }

        return "";
    }

    std::string FileSystem::GetExtensionFromFilePath(const std::string& filePath)
    {
        std::string extension;

        try
        {
            extension = std::filesystem::path(filePath).extension().generic_string(); // A system error is possible if the characters are something that can't be converted, like Russian.
        }
        catch (std::system_error& error)
        {
            AURORA_WARNING(LogLayer::Engine, "Failed. %s.", error.what());
        }

        return extension;
    }

    std::string FileSystem::GetFileNameFromFilePath(const std::string& filePath)
    {
        return std::filesystem::path(filePath).filename().generic_string();
    }

    std::string FileSystem::GetFileNameWithoutExtensionFromFilePath(const std::string& filePath)
    {
        const std::string fileName = GetFileNameFromFilePath(filePath);
        const size_t lastIndex = fileName.find_last_of('.');

        if (lastIndex != std::string::npos)
        {
            return fileName.substr(0, lastIndex);
        }

        return "";
    }

    std::string FileSystem::GetFilePathWithoutExtension(const std::string& filePath)
    {
        return GetDirectoryFromFilePath(filePath) + GetFileNameWithoutExtensionFromFilePath(filePath);
    }

    std::vector<std::string> FileSystem::GetDirectoriesInDirectory(const std::string& directoryPath)
    {
        std::vector<std::string> directories;
        const std::filesystem::directory_iterator iteratorEnd; // A default constructed directory_iterator is also known as the end iterator. See: https://en.cppreference.com/w/cpp/filesystem/directory_iterator/directory_iterator

        for (std::filesystem::directory_iterator iterator(directoryPath); iterator != iteratorEnd; ++iterator)
        {
            if (!std::filesystem::is_directory(iterator->status()))
            {
                continue;
            }

            std::string path;

            // A system_error is possible if the characters are something that cannot be converted, such as Russian.
            try
            {
                path = iterator->path().string();
            }
            catch (std::system_error& error)
            {
                AURORA_WARNING(Aurora::LogLayer::Engine, "Failed to read a directory path \"%s\". %s", path.c_str(), error.what());
            }

            if (!path.empty())
            {
                // Finally, let's save its path.
                directories.emplace_back(path);
            }
        }

        return directories;
    }

    std::vector<std::string> FileSystem::GetFilesInDirectory(const std::string& directoryPath)
    {
        std::vector<std::string> filePaths;
        const std::filesystem::directory_iterator iteratorEnd; 

        for (std::filesystem::directory_iterator iterator(directoryPath); iterator != iteratorEnd; ++iterator)
        {
            if (!std::filesystem::is_regular_file(iterator->status()))
            {
                continue;
            }

            std::string path;

            // A system_error is possible if the characters are something that cannot be converted, such as Russian.
            try
            {
                path = iterator->path().string();
            }
            catch (std::system_error& error)
            {
                AURORA_WARNING(Aurora::LogLayer::Engine, "Failed to read a directory path \"%s\". %s", path.c_str(), error.what());
            }

            if (!path.empty())
            {
                // Finally, let's save its path.
                filePaths.emplace_back(iterator->path().string());
            }
        }

        return filePaths;
    }

    void FileSystem::OpenItem(const std::string& directoryPath)
    {
        // See: https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutea
        ShellExecute(nullptr, nullptr, StringToWString(directoryPath).c_str(), nullptr, nullptr, SW_SHOW);
    }

    std::string FileSystem::GetWorkingDirectory()
    {
        return std::filesystem::current_path().generic_string();
    }

    std::string FileSystem::ReplaceOrAddExtension(const std::string& filePath, const std::string& fileExtension)
    {
        std::string path = filePath;
        size_t key = path.rfind('.');

        if (key != std::string::npos)
        {
            return (path.substr(0, key + 1) + fileExtension);
        }
        else
        {
            return (path + fileExtension);
        }

        return 0;
    }

    std::string FileSystem::RemoveIllegalCharacters(const std::string& filePath)
    {
        std::string legalText = filePath;

        // Remove characters that are illegal.
        std::string illegalCharacters = ":?\"<>|";
        for (auto iterator = legalText.begin(); iterator < legalText.end(); ++iterator)
        {
            if (illegalCharacters.find(*iterator) != std::string::npos)
            {
                *iterator = '_';
            }
        }

        return legalText;
    }

    std::string FileSystem::MakePathAbsolute(const std::string& filePath)
    {
        std::filesystem::path path = filePath;
        std::filesystem::path absolutePath = std::filesystem::absolute(path);

        if (!absolutePath.empty())
        {
            return absolutePath.string();
        }

        return "Null";
    }

    // Relative paths are paths that are in respect to another point in the file system. In our case, it is to our working directory.
    std::string FileSystem::GetRelativePath(const std::string& filePath)
    {
        if (std::filesystem::path(filePath).is_relative())
        {
            return filePath;
        }

        // Create absolute paths.
        const std::filesystem::path absolutePath = std::filesystem::absolute(filePath);
        const std::filesystem::path relativePath = std::filesystem::absolute(GetWorkingDirectory());

        // If the root paths are different, return absolute path.
        if (absolutePath.root_path() != relativePath.root_path())
        {
            return absolutePath.generic_string();
        }

        // Initialize relative path.
        std::filesystem::path result;

        // Find out where the two paths diverge.
        std::filesystem::path::const_iterator iteratorAbsolute = absolutePath.begin();
        std::filesystem::path::const_iterator iteratorRelative = relativePath.begin();

        // This loop will break once a discrepency is found between the paths or if either one ends.
        while (*iteratorAbsolute == *iteratorRelative && iteratorAbsolute != absolutePath.end() && iteratorRelative != relativePath.end())
        {
            ++iteratorAbsolute;
            ++iteratorRelative;
        }

        // Add "../" for each remaining token in iteratorRelativeTo.
        if (iteratorRelative != relativePath.end())
        {
            ++iteratorRelative;
            while (iteratorRelative != relativePath.end())
            {
                result /= "..";
                ++iteratorRelative;
            }
        }

        // Add remaining path.
        while (iteratorAbsolute != absolutePath.end())
        {
            result /= *iteratorAbsolute;
            ++iteratorAbsolute;
        }

        return result.generic_string();
    }

    std::string FileSystem::NativizeFilePath(const std::string& filePath)
    {
        const std::string filePathWithoutExtension = GetFilePathWithoutExtension(filePath);

        if (IsSupportedModelFile(filePath)) { return filePathWithoutExtension + EXTENSION_MODEL; }
        if (IsSupportedImageFile(filePath)) { return filePathWithoutExtension + EXTENSION_TEXTURE; }
        if (IsSupportedAudioFile(filePath)) { return filePathWithoutExtension + EXTENSION_AUDIO; }

        AURORA_WARNING(LogLayer::Engine, "Failed to nativize file path.");
        return filePath;
    }

    bool FileSystem::PushFileDataToBuffer(const std::string& fileName, std::vector<uint8_t>& data)
    {
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            size_t dataSize = (size_t)file.tellg();
            file.seekg(0, file.beg);
            data.resize(dataSize);
            file.read((char*)data.data(), dataSize);
            file.close();
            return true;
        }

        AURORA_ERROR(LogLayer::Engine, "File not found: %s.", fileName.c_str());
        return false;
    }

    bool FileSystem::IsEngineMaterialFile(const std::string& filePath)
    {
        return GetExtensionFromFilePath(filePath) == EXTENSION_MATERIAL;
    }

    bool FileSystem::IsEngineSceneFile(const std::string& filePath)
    {
        return GetExtensionFromFilePath(filePath) == EXTENSION_SCENE;
    }

    bool FileSystem::IsEngineModelFile(const std::string& filePath)
    {
        return GetExtensionFromFilePath(filePath) == EXTENSION_MODEL;
    }

    bool FileSystem::IsEngineCacheFile(const std::string& filePath)
    {
        return GetExtensionFromFilePath(filePath) == EXTENSION_CACHE;
    }

    bool FileSystem::IsEngineAudioFile(const std::string& filePath)
    {
        return GetExtensionFromFilePath(filePath) == EXTENSION_AUDIO;
    }

    bool FileSystem::IsEngineTextureFile(const std::string& filePath)
    {
        return GetExtensionFromFilePath(filePath) == EXTENSION_TEXTURE;
    }

    bool FileSystem::IsSupportedImageFile(const std::string& filePath)
    {
        const std::string extension = GetExtensionFromFilePath(filePath);

        for (const std::string& format : g_Supported_Image_Formats)
        {
            if (extension == format || extension == ConvertToUppercase(format))
            {
                return true;
            }
        }

        if (GetExtensionFromFilePath(filePath) == EXTENSION_TEXTURE)
        {
            return true;
        }

        return false;
    }

    bool FileSystem::IsSupportedModelFile(const std::string& filePath)
    {
        const std::string extension = GetExtensionFromFilePath(filePath);

        for (const std::string& format : g_Supported_Model_Formats)
        {
            if (extension == format || extension == ConvertToUppercase(format))
            {
                return true;
            }
        }

        return false;
    }

    bool FileSystem::IsSupportedAudioFile(const std::string& filePath)
    {
        const std::string extension = GetExtensionFromFilePath(filePath);

        for (const std::string& format : g_Supported_Audio_Formats)
        {
            if (extension == format || extension == ConvertToUppercase(format))
            {
                return true;
            }
        }

        return false;
    }

    bool FileSystem::IsSupportedResourceFile(const std::string& filePath)
    {
        return IsSupportedImageFile(filePath) || IsSupportedModelFile(filePath) || IsSupportedAudioFile(filePath);
    }

    bool FileSystem::IsSupportedEngineFile(const std::string& filePath)
    {
        return IsEngineModelFile(filePath)    ||
               IsEngineMaterialFile(filePath) ||
               IsEngineCacheFile(filePath)    ||
               IsEngineTextureFile(filePath)  ||
               IsEngineAudioFile(filePath) ||
               IsEngineSceneFile(filePath);
    }
}