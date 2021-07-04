#include "Globals.hlsli"

// Outputs from vertex shader go here. Can be interpolated to pixel shader.
struct vs_out
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.
    float2 outTexCoord : TEXCOORD;
    float3 outNormal   : NORMAL;
};

Texture2D objectTexture : TEXTURE: register(t0);
SamplerState objectSamplerState : SAMPLER: register(s0);

float4 main(vs_out input) : SV_TARGET // Pixel shader entry point which must return a float4 RGBA color value. 
{
    float3 norm = normalize(input.outNormal);
    float3 lightDirection = normalize(g_Light_Position - input.outPosition);
    float3 diff = max(dot(norm, lightDirection), 0.0);
    float3 diffuse = diff * g_Light_Color;

    float3 sampleColor = objectTexture.Sample(objectSamplerState, input.outTexCoord);
    float3 ambientLight = g_Light_Color * 0.3f;

    float3 finalColor = sampleColor * (ambientLight + diffuse);

    return float4(finalColor, 1.0); // Must return an RGBA colour.
}