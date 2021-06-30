#define OBJECTSHADER_LAYOUT_POS_TEX
#define OBJECTSHADER_USE_COLOR
#include "ObjectHF.hlsli"

[earlydepthstencil]
float4 main(PixelInput input) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}