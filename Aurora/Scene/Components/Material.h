#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include "../Resource/AuroraResource.h"
#include "../_Shaders/Mappings/ResourceMappings.h"
#include <string>

using namespace DirectX;

namespace Aurora
{   
    // These texture slots correspond to our shader binding points.
    enum TextureSlot
    {
        BaseColorMap,
        NormalMap, 
        MetalnessMap,
        RoughnessMap,
        EmissiveMap,
        DisplacementMap,
        OcclusionMap,
        TransmissionMap,
        SpecularMap,
        TextureSlot_Count
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

        void Serialize(SerializationStream& outputStream) override;
        void Deserialize(SerializationNode& inputNode) override;

        XMFLOAT4 GetBaseColor() const { return m_BaseColor; }

        void SetBaseColor(const XMFLOAT4& newColor) { SetDirty(); m_BaseColor = newColor; }

        void SetDirty(bool value = true) { if (value) { m_Flags |= Material_Flags::Material_Flags_Dirty; } else { m_Flags &= ~Material_Flags::Material_Flags_Dirty; } }
        bool IsDirty() const { return m_Flags & Material_Flags::Material_Flags_Dirty; }

    public:
        std::shared_ptr<AuroraResource> m_Textures[TextureSlot::TextureSlot_Count];

    public:
        XMFLOAT4 m_BaseColor = XMFLOAT4(1, 1, 1, 1);
        float m_Roughness = 0.25;
        float m_Metalness = 0.7;

        uint32_t m_Flags = Material_Flags::Material_Flags_Empty;
    };
}