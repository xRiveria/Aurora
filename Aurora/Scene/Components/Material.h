#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include "../Resource/ResourceUtilities.h"
#include <string>

using namespace DirectX;

namespace Aurora
{   
    enum TextureSlot
    {
        BaseColorMap,
        NormalMap,
        SurfaceMap,
        EmissiveMap,
        DisplacementMap,
        OcclusionMap,
        TransmissionMap,
        SpecularMap,
        TextureSlot_Count
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

    enum Material_Flags
    {
        Material_Flags_Empty = 0,
        Material_Flags_Dirty = 1 << 0
    };

    class Material : public IComponent
    {
    public:
        Material(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);

        XMFLOAT4 GetBaseColor() const { return m_BaseColor; }

        void SetBaseColor(const XMFLOAT4& newColor) { SetDirty(); m_BaseColor = newColor; }

        void SetDirty(bool value = true) { if (value) { m_Flags |= Material_Flags::Material_Flags_Dirty; } else { m_Flags &= ~Material_Flags::Material_Flags_Dirty; } }
        bool IsDirty() const { return m_Flags & Material_Flags::Material_Flags_Dirty; }

    public:
        TextureMap m_Textures[TextureSlot::TextureSlot_Count];

    private:
        XMFLOAT4 m_BaseColor = XMFLOAT4(1, 1, 1, 1);

        uint32_t m_Flags = Material_Flags::Material_Flags_Empty;
    };
}