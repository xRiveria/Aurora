#pragma once
#include "ShaderUtilities.h"

/* Personal Notes

    We are utilizing a slot based binding model. However, it does have a CPU overhead. Avoid binding massive amount of resources. The maximum slot numbers that should be used are:
    
    - 15 for BindConstantBuffer(),
    - 64 for BindResource(),
    - 8  for BindUAV()
*/

// On Demand Constant Buffers

CBUFFER(Misc_ConstantBuffer, CBSLOT_RENDERER_MISC)
{
    float4x4 g_Transform;
    float4   g_Color;
};