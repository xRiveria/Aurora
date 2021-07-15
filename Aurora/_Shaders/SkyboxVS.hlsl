#include "Globals.hlsli"

struct PixelShaderInput
{
    float3 localPosition : POSITION;
    float4 pixelPosition : SV_POSITION;
};

PixelShaderInput main(float3 position : POSITION)
{
    PixelShaderInput output;
    output.localPosition = position;
    output.pixelPosition = mul(g_Camera_ViewProjection, float4(position, 1.0));

    return output;
}