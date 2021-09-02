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
        static bool IsEmptyOrWhitespace(const std::string& line);
        static bool CreateDirectory_(const std::string& directoryPath);
        static bool Exists(const std::string& filePath);
        static bool Delete(const std::string& filePath);
        static bool IsDirectory(const std::string& filePath);
        static bool IsFile(const std::string& filePath);
        static void OpenDirectoryWindow(const std::string& directory);

        static std::string GetDirectoryFromFilePath(const std::string& filePath);
        static std::string GetExtensionFromFilePath(const std::string& filePath);
        static std::string GetFileNameFromFilePath(const std::string& filePath);
        static std::string GetFileNameWithoutExtensionFromFilePath(const std::string& filePath);
        static std::string GetFilePathWithoutExtension(const std::string& filePath);
        static std::string ReplaceExtension(const std::string& filePath, const std::string& extension);
        static std::vector<std::string> GetDirectoriesInDirectory(const std::string& directoryPath);
        static std::vector<std::string> GetFilesInDirectory(const std::string& directoryPath);
        static bool CopyFileFromTo(const std::string& source, const std::string& destination);

        static void OpenItem(const std::string& directoryPath);
        static std::string GetWorkingDirectory();
        static std::string ReplaceOrAddExtension(const std::string& filePath, const std::string& fileExtension);
        static std::string RemoveIllegalCharacters(const std::string& filePath);
        
        // Path Absolute
        static std::string MakePathAbsolute(const std::string& filePath);
        static std::string GetRelativePath(const std::string& filePath);
        static std::string NativizeFilePath(const std::string& filePath);

        // File Operations
        static bool PushFileDataToBuffer(const std::string& fileName, std::vector<uint8_t>& data);

        // Supported Files by the Engine
        static bool IsEngineMaterialFile(const std::string& filePath);
        static bool IsEngineSceneFile(const std::string& filePath);
        static bool IsEngineModelFile(const std::string& filePath);
        static bool IsEngineTextureFile(const std::string& filePath);
        static bool IsEngineCacheFile(const std::string& filePath);
        static bool IsEngineAudioFile(const std::string& filePath);
        static bool IsSupportedEngineFile(const std::string& filePath);

        static bool IsSupportedAudioFile(const std::string& filePath);
        static bool IsSupportedImageFile(const std::string& filePath);
        static bool IsSupportedModelFile(const std::string& filePath);
        static bool IsSupportedResourceFile(const std::string& filePath);
    };

    static const char* EXTENSION_AUDIO = ".audio";
    static const char* EXTENSION_TEXTURE = ".texture";
    static const char* EXTENSION_MODEL = ".model";
    static const char* EXTENSION_SCENE = ".aurora";
    static const char* EXTENSION_MATERIAL = ".material";
    static const char* EXTENSION_CACHE = ".dat";

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

    static const std::vector<std::string> g_Supported_Audio_Formats
    {
        ".aiff",
        ".asf",
        ".asx",
        ".dls",
        ".flac",
        ".fsb",
        ".it",
        ".m3u",
        ".midi",
        ".mod",
        ".mp2",
        ".mp3",
        ".ogg",
        ".pls",
        ".s3m",
        ".vag", // PS2/PSP
        ".wav",
        ".wax",
        ".wma",
        ".xm",
        ".xma" // XBOX 360
    };
}