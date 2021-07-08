#pragma once
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;

namespace Aurora
{
    class AuroraColor
    {
    public:
        uint32_t m_RGBA = 0;

    public:
        constexpr AuroraColor(uint32_t rgba) : m_RGBA(rgba) {}
        constexpr AuroraColor(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255) : m_RGBA((r << 0) | (g << 8) | (b << 16) | (a << 24)) {}

        constexpr uint8_t GetRChannel() const { return (m_RGBA >> 0) & 0xFF; }
        constexpr uint8_t GetGChannel() const { return (m_RGBA >> 8) & 0xFF; }
        constexpr uint8_t GetBChannel() const { return (m_RGBA >> 16) & 0xFF; }
        constexpr uint8_t GetAChannel() const { return (m_RGBA >> 24) & 0xFF; }

        constexpr void SetRChannel(uint8_t value) { *this = AuroraColor(value, GetGChannel(), GetBChannel(), GetAChannel()); }
        constexpr void SetGChannel(uint8_t value) { *this = AuroraColor(GetRChannel(), value, GetBChannel(), GetAChannel()); }
        constexpr void SetBChannel(uint8_t value) { *this = AuroraColor(GetRChannel(), GetGChannel(), value, GetAChannel()); }
        constexpr void SetAChannel(uint8_t value) { *this = AuroraColor(GetRChannel(), GetGChannel(), GetBChannel(), value); }
        
        // Returns a value between 0 and 1.
        constexpr XMFLOAT3 ToFloat3() const
        {
            return XMFLOAT3(
                ((m_RGBA >> 0)  & 0xFF)  / 255.0f,
                ((m_RGBA >> 8)  & 0xFF)  / 255.0f,
                ((m_RGBA >> 16) & 0xFF)  / 255.0f
            );
        }

        constexpr operator XMFLOAT3() const { return ToFloat3(); }

        constexpr XMFLOAT4 ToFloat4() const
        {
            return XMFLOAT4(
                ((m_RGBA >> 0)  & 0xFF) / 255.0f,
                ((m_RGBA >> 8)  & 0xFF) / 255.0f,
                ((m_RGBA >> 16) & 0xFF) / 255.0f,
                ((m_RGBA >> 24) & 0xFF) / 255.0f
            );
        }

        constexpr operator XMFLOAT4() const { return ToFloat4(); }

        static constexpr AuroraColor Red()         { return AuroraColor(255, 0, 0, 255); }
        static constexpr AuroraColor Green()       { return AuroraColor(0, 255, 0, 255); }
        static constexpr AuroraColor Blue()        { return AuroraColor(0, 0, 255, 255); }
        static constexpr AuroraColor Black()       { return AuroraColor(0, 0, 0, 255); }
        static constexpr AuroraColor White()       { return AuroraColor(255, 255, 255, 255); }
        static constexpr AuroraColor Yellow()      { return AuroraColor(255, 255, 0, 255); }
        static constexpr AuroraColor Purple()      { return AuroraColor(255, 0, 255, 255); }
        static constexpr AuroraColor Cyan()        { return AuroraColor(0, 255, 255, 255); }
        static constexpr AuroraColor Transparent() { return AuroraColor(0, 0, 0, 0); }
        static constexpr AuroraColor Gray()        { return AuroraColor(127, 127, 127, 255); }
    };
}