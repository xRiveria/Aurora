#pragma once
#include <memory>
#include <string>
#include "../Resource/ResourceUtilities.h"

namespace Aurora
{
    enum Weather_Flags
    {
        Weather_Flag_Empty              = 0,
        Weather_Flag_Simple_Sky         = 1 << 0,
        Weather_Flag_Realistic_Sky      = 1 << 1,
        Weather_Flag_Volumetric_Clouds  = 1 << 2
    };

    class Weather
    {
    public:
        Weather() = default;
        ~Weather() = default;

        void SetPreset_Cloudy();
        
    public:
        XMFLOAT3 m_SunColor = XMFLOAT3(0, 0, 0);
        XMFLOAT3 m_SunDirection = XMFLOAT3(0, 1, 0);
        float m_SkyEnergy = 0.0f;
        float m_SkyExposure = -1.0f;

        // == Sky ==
        XMFLOAT3 m_HorizonColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMFLOAT3 m_ZenithColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMFLOAT3 m_AmbientColor = XMFLOAT3(0.2f, 0.2f, 0.2f);
        // https://lh3.googleusercontent.com/proxy/VADoPVmJoJAkqkNK-TnSFqUlZJ4dUDAN_1WU3zUowMNmGwuhy-2kGW9a1Tz2vDJt014kUw0pe6_g1SrmiXbtCOmrDwE_XxPpmgAAgzl1G3gyM8XqNYyA

        // == Fog ==
        float m_FogStart = 100.0f;
        float m_FogEnd = 1000.0f;
        float m_FogHeight = 0.0f;

        // == Clouds ==
        float m_Cloudiness = 0.0f;
        float m_CloudScale = 0.0003f;
        float m_CloudSpeed = 0.1f;

        std::string m_SkyMapName = "Empty Sky Map";
        std::shared_ptr<AuroraResource> m_SkyMapTexture = nullptr;

        uint32_t m_WeatherFlags = 0;
    };
}