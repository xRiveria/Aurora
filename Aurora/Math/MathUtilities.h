#pragma once
#include "Core.h"
#include "../Graphics/RHI_Implementation.h"

using namespace DirectX;

namespace Aurora::Math
{
    constexpr XMFLOAT3 Minimum(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return XMFLOAT3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
    }

    constexpr XMFLOAT3 Maximum(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return XMFLOAT3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
    }

    constexpr float Lerp(float value1, float value2, float amount)
    {
        return value1 + (value2 - value1) * amount;
    }

    constexpr XMFLOAT2 Lerp(const XMFLOAT2& value1, const XMFLOAT3& value2, float i)
    {
        return XMFLOAT2(Lerp(value1.x, value2.x, i), Lerp(value1.y, value2.y, i));
    }

    constexpr XMFLOAT3 Lerp(const XMFLOAT3& value1, const XMFLOAT3& value2, float i)
    {
        return XMFLOAT3(Lerp(value1.x, value2.x, i), Lerp(value1.y, value2.y, i), Lerp(value1.z, value2.z, i));
    }

    constexpr XMFLOAT4 Lerp(const XMFLOAT4& value1, const XMFLOAT4& value2, float i)
    {
        return XMFLOAT4(Lerp(value1.x, value2.x, i), Lerp(value1.y, value2.y, i), Lerp(value1.z, value2.z, i), Lerp(value1.w, value2.w, i));
    }

    constexpr float Clamp(float value, float minimum, float maximum)
    {
        if (value < minimum) 
        { 
            return minimum; 
        }
        else if (value > maximum)
        {
            return maximum;
        }

        return value;
    }
}