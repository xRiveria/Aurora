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
        static std::string GetDirectoryFromFilePath(const std::string& filePath);
        static std::string GetExtensionFromFilePath(const std::string& filePath);
        static std::string GetFileNameFromFilePath(const std::string& filePath);
        static std::string GetFileNameWithoutExtensionFromFilePath(const std::string& filePath);
        static std::string GetFilePathWithoutExtension(const std::string& filePath);
        static std::string ReplaceExtension(const std::string& filePath, const std::string& fileExtension);
        
        // Path Absolute
        static std::string MakePathAbsolute(const std::string& filePath);

        // File Operations
        static bool PushFileDataToBuffer(const std::string& fileName, std::vector<uint8_t>& data);

        // Supported Files in Directory
        static bool IsSupportedImageFile(const std::string& filePath);
    };

    static const char* EXTENSION_TEXTURE = ".texture";

    static const std::vector<std::string> g_Supported_Image_Formats
    {
        ".jpg",
        ".png",
        ".bmp",
        ".tga",
        ".dds",
        ".exr",
        ".raw",
        ".gif",
        ".hdr",
        ".ico",
        ".iff",
        ".jng",
        ".jpeg",
        ".koala",
        ".kodak",
        ".mng",
        ".pcx",
        ".pbm",
        ".pgm",
        ".ppm",
        ".pfm",
        ".pict",
        ".psd",
        ".raw",
        ".sgi",
        ".targa",
        ".tiff",
        ".tif", 
        ".wbmp",
        ".webp",
        ".xbm",
        ".xpm"
    };
}