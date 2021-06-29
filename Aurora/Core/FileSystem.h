#pragma once
#include <vector>
#include <string>
#include "Core.h"

namespace Aurora
{
    class FileSystem
    {
    public:
        // Strings
        static std::string ConvertToUppercase(const std::string& lowerString);
        static std::wstring StringToWString(const std::string& string);
        static std::string WStringToString(const std::wstring& wString);

        // Paths
        static std::string GetExtensionFromFilePath(const std::string& filePath);
        static std::string GetNameFromFilePath(const std::string& filePath);

        // Supported Files in Directory
    };

    static const std::vector<std::string> g_Supported_Image_Formats
    {

    };
}