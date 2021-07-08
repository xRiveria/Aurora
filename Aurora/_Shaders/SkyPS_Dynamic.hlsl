#include "Globals.hlsli"
#include "SkyHF.hlsli"

float4 main(float4 position : SV_POSITION, float2 clipSpace : TEXCOORD) : SV_TARGET
{
    float4 unprojected = mul(g_Camera_InverseViewProjection, float4(clipSpace, 0.0f, 1.0f));
    unprojected.xyz /= unprojected.w;

    const float3 view = normalize(unprojected.xyz - g_Camera_Position);

    float4 color = float4(GetDynamicSkyColor(view), 1);

    return color;
}