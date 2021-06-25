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
}