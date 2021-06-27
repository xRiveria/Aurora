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
}