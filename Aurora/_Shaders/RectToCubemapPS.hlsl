#include "Globals.hlsli"
#include "Mappings.hlsli"

SamplerState objectSamplerState : SAMPLER: register(s0);

struct InputPixelType
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.
    float3 localPosition : TEXCOORD1;
};

static const float inverseAtan = float2(0.1591, 0.3183);

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= inverseAtan;
    uv += 0.5;
    return uv;
}

float4 main(InputPixelType input) : SV_TARGET
{
    float2 uv = SampleSphericalMap(normalize(input.localPosition));
    float3 color = Texture_SkyHDRMap.Sample(objectSamplerState, uv).rgb;

    return float4(color, 1.0);
}