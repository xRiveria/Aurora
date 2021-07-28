#pragma once
#include "IComponent.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace Aurora
{
    class Light : public IComponent
    {
    public:
        Light(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~Light();

        enum Lighting_Flags
        {
            Empty = 0,
            Cast_Shadow = 1 << 0,
        };

        uint32_t m_Flags = Lighting_Flags::Empty;
        XMFLOAT3 m_Color = XMFLOAT3(1, 1, 1);

        enum Lighting_Type
        {
            Directional = 0,   // 0
            Point       = 1,         // 1
            Spot        = 2,          // 2
            LightingType_Count
        };

        
        Lighting_Type m_Type = Lighting_Type::Point;
        // float m_Energy = 1.0f;
        // float m_RangeLocal = 10.0f;
        // float m_FOV = XM_PIDIV4;

        float m_RangeGlobal;
        XMFLOAT3 m_Direction = { -2.0f, -1.0f, -0.3f };
        XMFLOAT3 m_Rotation;
        XMFLOAT3 m_Scale;
        XMFLOAT3 m_Front;
        XMFLOAT3 m_Right;
        float m_Intensity = 1.0;

    public:
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

        void SetLightingType(Lighting_Type type) { m_Type = type; }
        Lighting_Type GetType() const { return m_Type; }
    };
}