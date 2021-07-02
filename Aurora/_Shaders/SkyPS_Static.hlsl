#include "Globals.hlsli"

SamplerState objectSamplerState : SAMPLER: register(s0);

float4 main(float4 position : SV_POSITION, float2 clipSpace : TEXCOORD) : SV_TARGET
{
    float4 unprojected = mul(g_Camera_InverseViewProjection, float4(clipSpace, 0.0f, 1.0f));
    unprojected.xyz /= unprojected.w;

    const float3 view = normalize(unprojected.xyz - g_Camera_Position);

    float4 color = float4(texture_global_environmental_map.SampleLevel(objectSamplerState, view, 0).rgb, 1);

    return color;
}