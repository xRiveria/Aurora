#pragma once
#include "../Graphics/RHI_Implementation.h"

using namespace DirectX;

namespace Aurora
{
    enum Texture_Slot
    {
        BaseColor_Map,
        Normal_Map,
        Surface_Map,
        Emissive_Map,
        Displacement_Map,
        Occlusion_Map,
        Transmission_Map,
        SheenColor_Map,
        SheenRoughness_Map,
        ClearCoat_Map,
        ClearCoatRoughness_Map,
        ClearCoatNormal_Map,
        Specular_Map,

        Texture_Slot_Count
    };

    struct Texture_Map
    {
        std::string m_Name;
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

    class Material
    {

    public:
        XMFLOAT4 m_BaseColor = XMFLOAT4(1, 1, 1, 1);
        XMFLOAT4 m_SpecularColor = XMFLOAT4(1, 1, 1, 1);
        XMFLOAT4 m_EmissiveColor = XMFLOAT4(1, 1, 1, 0);
        float m_Roughness = 0.2f;
        float m_Reflectance = 0.02f;
        float m_Metalness = 0.0f;
        float m_NormalMapStrength = 1.0f;
        float m_DisplacementMapping = 0.0f;
        float m_Refraction = 0.0f;
        float m_Transmission = 0.0f;
        float m_AlphaReference = 1.0f;

        Texture_Map m_Textures[Texture_Slot::Texture_Slot_Count];

        // Non Serialized Attributes
        RHI_GPU_Buffer m_ConstantBuffer;
    };
}