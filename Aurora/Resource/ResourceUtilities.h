#pragma once
#include "../Graphics/RHI_GraphicsDevice.h"

namespace Aurora
{
    enum Resource_Data_Type
    {
        Empty,
        Image,
        Sound
    };

    enum Resource_Load_Mode
    {
        Import_Discard_File_Data_After_Load,                   // Default behavior. The file data will be discarded after loading. This will not allow serialization of embedded resources. Less memory will be used overall.
        Import_Allow_Retain_File_Data,                         // Allows for the file data to be kept. This mode allows serialization of embedded resources which request it.
        Import_Allow_Retain_File_Data_Without_Embedding        // Allows keeping of file data, but it won't be embedded by the serializer.
    };

    enum Resource_Flags
    {
        Import_Color_Grading_LUT = 1 << 0,  // Image import will convert the resource to 3D color grading LUT.
        Import_Retain_File_Data  = 1 << 1   // File data will be kept for later reuse. This is required for keeping the resourced serializable. 
    };

    // Audio files in the far future can be included as well.
    struct AuroraResource
    {
        Aurora::RHI_Texture m_Texture;

        Resource_Data_Type m_Type = Resource_Data_Type::Empty;

        uint32_t m_Flags = 0;
        std::vector<uint8_t> m_FileData;
    };
}