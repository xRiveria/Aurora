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

        // Paths
        static std::string GetExtensionFromFilePath(const std::string& filePath);
    };
}