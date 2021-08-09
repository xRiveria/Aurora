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

    std::string FileSystem::MakePathAbsolute(const std::string& filePath)
    {
        std::filesystem::path path = filePath;
        std::filesystem::path absolutePath = std::filesystem::absolute(path);

        if (!absolutePath.empty())
        {
            return absolutePath.string();
        }

        return 0;
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

    bool FileSystem::IsSuppportedModelFile(const std::string& filePath)
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
}