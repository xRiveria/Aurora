#include "Globals.hlsli"

struct PixelShaderInput
{
    float3 localPosition : POSITION;
    float4 pixelPosition : SV_POSITION;
};

SamplerState objectSamplerState : SAMPLER: register(s0);

float4 main(PixelShaderInput pixelInput) : SV_TARGET
{
    return float4(1, 1, 1, 1);
    // float3 environmentVector = normalize(pixelInput.localPosition);
    // return g_EnvironmentTexture.SampleLevel(defaultSampler, environmentVector, 0);
}