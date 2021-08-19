#define SHADER_LAYOUT_POSITION_COLOR
#define SHADER_USE_POSITION_COLOR
#include "Common.hlsli"

PixelInput main(VertexInput input)
{
    PixelInput output;

    float4 position = float4(input.position, 1.0f);
    output.position = mul(g_Camera_ViewProjection, position);
    output.color = input.color;

    return output;
}