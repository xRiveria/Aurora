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
        static bool CreateDirectory_(const std::string& directoryPath);
        static bool Exists(const std::string& filePath);
        static bool Delete(const std::string& filePath);
        static bool IsDirectory(const std::string& filePath);
        static std::string GetDirectoryFromFilePath(const std::string& filePath);
        static std::string GetExtensionFromFilePath(const std::string& filePath);
        static std::string GetFileNameFromFilePath(const std::string& filePath);
        static std::string GetFileNameWithoutExtensionFromFilePath(const std::string& filePath);
        static std::string GetFilePathWithoutExtension(const std::string& filePath);
        static std::vector<std::string> GetDirectoriesInDirectory(const std::string& directoryPath);
        static std::vector<std::string> GetFilesInDirectory(const std::string& directoryPath);

        static void OpenItem(const std::string& directoryPath);
        static std::string GetWorkingDirectory();
        static std::string ReplaceOrAddExtension(const std::string& filePath, const std::string& fileExtension);
        
        // Path Absolute
        static std::string MakePathAbsolute(const std::string& filePath);

        // File Operations
        static bool PushFileDataToBuffer(const std::string& fileName, std::vector<uint8_t>& data);

        // Supported Files in Directory
        static bool IsSupportedImageFile(const std::string& filePath);
        static bool IsSuppportedModelFile(const std::string& filePath);
        static bool IsEngineFile(const std::string& filePath);
    };

    static const char* EXTENSION_TEXTURE = ".texture";
    static const char* EXTENSION_MODEL = ".model";
    static const char* EXTENSION_SCENE = ".aurora";

    static const std::vector<std::string> g_Supported_Model_Formats
    {
        ".3ds",
        ".obj",
        ".fbx",
        ".blend",
        ".dae",
        ".gltf",
        ".lwo",
        ".c4d",
        ".ase",
        ".dxf",
        ".hmp",
        ".md2",
        ".md3",
        ".md5",
        ".mdc",
        ".mdl",
        ".nff",
        ".ply",
        ".stl",
        ".x",
        ".smd",
        ".lxo",
        ".lws",
        ".ter",
        ".ac3d",
        ".ms3d",
        ".cob",
        ".q3bsp",
        ".xgl",
        ".csm",
        ".bvh",
        ".b3d",
        ".ndo"
    };

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