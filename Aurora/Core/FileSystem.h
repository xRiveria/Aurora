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
        static bool Exists(const std::string& filePath);
        static std::string GetExtensionFromFilePath(const std::string& filePath);
        static std::string GetNameFromFilePath(const std::string& filePath);
        static std::string ReplaceExtension(const std::string& filePath, const std::string& fileExtension);
        
        // Path Absolute
        static std::string MakePathAbsolute(const std::string& filePath);

        // File Operations
        static bool PushFileDataToBuffer(const std::string& fileName, std::vector<uint8_t>& data);

        // Supported Files in Directory
    };

    static const std::vector<std::string> g_Supported_Image_Formats
    {

    };
}