#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include "../Graphics/DX11_Refactored/DX11_Texture.h"

using namespace DirectX;

namespace Aurora
{
    class Renderer;

    enum Lighting_Type
    {
        Directional = 0,            
        Point       = 1,                  
        Spot        = 2,                   
        LightingType_Count
    };

    struct ShadowSlice
    {
        
    };

    struct ShadowMap
    {
        std::shared_ptr<DX11_Texture> m_ColorTexture;
        std::shared_ptr<DX11_Texture> m_DepthTexture;
        std::vector<ShadowSlice> m_Slices;
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

        void Tick(float deltaTime) override;
        // Serialization
        void Serialize(BinarySerializer* binarySerializer) override;
        void Deserialize(BinarySerializer* binaryDeserializer) override;

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

    private:
        void ComputeViewMatrix();
        void CreateShadowMap();
        void ComputeCascadeSplits();

    private:
        bool m_IsInitialized = false;
        Renderer* m_Renderer = nullptr;

        // Shadows
        uint32_t m_CascadeCount = 4;
        ShadowMap m_ShadowMap;
        uint32_t m_Flags = Lighting_Flags::Lighting_Flags_Empty;
    };
}