#pragma once
#include "../Resource/ResourceUtilities.h"
#include "../Renderer/RendererEnums.h"

namespace Aurora
{
    enum Material_Flags
    {
        Material_Flag_Empty = 0,
        Material_Flag_Dirty = 1 << 0,
        Material_Flag_Cast_Shadow = 1 << 1,
        Material_Flag_Use_VertexColors = 1 << 5,
        Material_Flag_Disable_Receive_Shadow = 1 << 10,
        Material_Flag_Double_Sided = 1 << 11
    };

    enum Material_Shader_Type
    {
        Material_Shader_Type_PBR,
        Material_Shader_Type_Count
    };

    enum Texture_Slot
    {
        Base_Color_Map,
        Normal_Map,
        Surface_Map,
        Emissive_map,
        Displacement_Map,
        Occlusion_Map,
        Transmission_Map,  // Thickness
        Specular_Map,

        Texture_Slot_Count
    };

    struct TextureMap
    {
        std::string m_FilePath;
        uint32_t m_UVSet = 0;
        std::shared_ptr<AuroraResource> m_Resource;

        const RHI_GPU_Resource* GetGPUResource() const
        {
            if (m_Resource == nullptr || !m_Resource->m_Texture.IsValid())
            {
                return nullptr;
            }
            return &m_Resource->m_Texture;
        }

        int GetUVSet() const
        {
            if (m_Resource == nullptr || !m_Resource->m_Texture.IsValid())
            {
                return -1;
            }

            return (int)m_UVSet;
        }
    };
}