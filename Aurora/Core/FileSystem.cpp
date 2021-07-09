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
            AURORA_WARNING("%s, %s", error.what(), filePath.c_str());
        }

        return false;
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
            AURORA_WARNING("Failed. %s.", error.what());
        }

        return extension;
    }

    std::string FileSystem::GetNameFromFilePath(const std::string& filePath)
    {
        return std::filesystem::path(filePath).filename().generic_string();
    }

    std::string FileSystem::GetDirectoryFromFilePath(const std::string& filePath)
    {
        const size_t lastSlashIndex = filePath.find_last_of("\\/"); // Note that find_last_of searches for any of the characters in the arguments. Hence, we use forward and backward slash as both are valid path seperators on Windows.

        if (lastSlashIndex != std::string::npos)
        {
            return filePath.substr(0, lastSlashIndex + 1);
        }

        return "";
    }

    std::string FileSystem::ReplaceExtension(const std::string& filePath, const std::string& fileExtension)
    {
        std::string path = filePath;
        size_t key = path.rfind('.');

        if (key != std::string::npos)
        {
            return (path.substr(0, key + 1) + fileExtension);
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

    bool FileSystem::PushFileDataToBuffer(const std::string& filePath, std::vector<uint8_t>& data)
    {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            size_t dataSize = (size_t)file.tellg();
            file.seekg(0, file.beg);
            data.resize(dataSize);
            file.read((char*)data.data(), dataSize);
            file.close();
            return true;
        }

        AURORA_ERROR("File not found: %s.", filePath.c_str());
        return false;
    }
}