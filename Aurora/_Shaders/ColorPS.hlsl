#define SHADER_LAYOUT_POSITION_COLOR
#define SHADER_USE_POSITION_COLOR
#include "Common.hlsli"

float4 main(PixelInput pixelInput) : SV_TARGET
{
    return pixelInput.color;
}