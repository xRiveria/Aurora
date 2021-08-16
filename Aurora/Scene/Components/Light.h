#pragma once
#include "IComponent.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace Aurora
{
    enum Lighting_Type
    {
        Directional = 0,            // 0
        Point = 1,                  // 1
        Spot = 2,                   // 2
        LightingType_Count
    };

    enum Lighting_Flags
    {
        Lighting_Flags_Empty = 0,
        Cast_Shadow = 1 << 0,
    };

    class Light : public IComponent
    {
    public:
        Light(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~Light();

        void Serialize(SerializationStream& outputStream) override;
        void Deserialize(SerializationNode& inputNode) override;

        void SetLightingType(Lighting_Type type) { m_LightType = type; }
        Lighting_Type GetLightingType() const { return m_LightType; }

        void SetIsCastingShadow(bool value) 
        { 
            if (value) 
            {
                m_Flags |= Lighting_Flags::Cast_Shadow; 
            } 
            else
            {
                m_Flags &= ~Lighting_Flags::Cast_Shadow;
            }
        }
        bool IsCastingShadow() const { return m_Flags & Lighting_Flags::Cast_Shadow; }

    public:
        XMFLOAT3 m_Color = XMFLOAT3(1, 1, 1);
        float m_Intensity = 40.0;

        Lighting_Type m_LightType = Lighting_Type::Point;
        uint32_t m_Flags = Lighting_Flags::Lighting_Flags_Empty;
    };
}