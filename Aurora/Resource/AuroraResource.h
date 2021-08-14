#pragma once
#include "../Resource/AuroraObject.h"
#include "../Scene/Entity.h"
#include "../Graphics/DX11_Refactored/DX11_Texture.h"
#include "../Graphics/DX11_Refactored/DX11_VertexBuffer.h"
#include "../Graphics/DX11_Refactored/DX11_IndexBuffer.h"
#include <memory>
#include <variant>

namespace Aurora
{
    enum Resource_Type
    {
        ResourceType_Empty,
        ResourceType_Image,
        ResourceType_Audio,
        ResourceType_Model
    };

    // A struct holding crucial information regarding a mesh/model's polygons, normals etc.
    struct DX11_MeshData
    {
        DX11_MeshData() = default;

        std::shared_ptr<DX11_VertexBuffer> m_VertexBuffer = nullptr;
        std::shared_ptr<DX11_IndexBuffer> m_IndexBuffer = nullptr;
    };

    class AuroraResource : public AuroraObject
    {
    public:
        // Aurora::RHI_Texture m_Texture;

        // ===================================================================
        std::string TypeToString()
        {
            switch (m_Type)
            {
                case Resource_Type::ResourceType_Empty:
                    return "Empty";

                case Resource_Type::ResourceType_Image:
                    return "Image";

                case Resource_Type::ResourceType_Audio:
                    return "Audio";

                case Resource_Type::ResourceType_Model:
                    return "Model";
                }

            return "Invalid Resource Type";
        }
   
        std::shared_ptr<DX11_Texture> m_Texture;
        std::vector<DX11_MeshData> m_Meshes; // Future types.
        Entity* m_Entity = nullptr; // Texture and Models involve entities. We store them here for reference.

        Resource_Type m_Type = Resource_Type::ResourceType_Empty;
        std::string m_FilePath = "Empty Path";
    };

    enum Resource_Load_Mode
    {
        Import_Discard_File_Data_After_Load,                   // Default behavior. The file data will be discarded after loading. This will not allow serialization of embedded resources. Less memory will be used overall.
        Import_Allow_Retain_File_Data,                         // Allows for the file data to be kept. This mode allows serialization of embedded resources which request it.
        Import_Allow_Retain_File_Data_Without_Embedding        // Allows keeping of file data, but it won't be embedded by the serializer.
    };

    enum Resource_Flags
    {
        Import_Color_Grading_LUT = 1 << 0,                     // Image import will convert the resource to 3D color grading LUT.
        Import_Retain_File_Data  = 1 << 1                      // File data will be kept for later reuse. This is required for keeping the resourced serializable. 
    };
}