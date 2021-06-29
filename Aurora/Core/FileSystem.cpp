#include "Aurora.h"
#include "FileSystem.h"
#include <filesystem>
#include <regex>
#include <Windows.h>
#include <shellapi.h>

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
}