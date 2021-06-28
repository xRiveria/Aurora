#pragma once
#include "ShaderUtilities.h"

// On Demand Constant Buffers

CBUFFER(Misc_ConstantBuffer, CBSLOT_RENDERER_MISC)
{
    float4x4 g_Transform;
    float4   g_Color;
};