#include "Globals.hlsli"

// Outputs from vertex shader go here. Can be interpolated to pixel shader.
struct vs_out
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.

    float2 outTexCoord : TEXCOORD;
    float3 outNormal   : NORMAL;
    float3 outWorldSpace : WORLD_POSITION;
};

Texture2D objectTexture : TEXTURE: register(t4);
SamplerState objectSamplerState : SAMPLER: register(s0);
float weight[5] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };

float4 main(vs_out input) : SV_TARGET
{
    float2 textureDimensions;
    objectTexture.GetDimensions(textureDimensions.x, textureDimensions.y);
    float2 texOffset = 1.0 / textureDimensions; // Returns the size of a single texel.

    float3 result = objectTexture.Sample(objectSamplerState, input.outTexCoord) * weight[0]; // Current fragment's contribution.
    if (g_IsHorizontalPass)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += objectTexture.Sample(objectSamplerState, input.outTexCoord + float2(texOffset.x * i, 0.0)) * weight[i];
            result += objectTexture.Sample(objectSamplerState, input.outTexCoord - float2(texOffset.x * i, 0.0)) * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += objectTexture.Sample(objectSamplerState, input.outTexCoord + float2(0.0, texOffset.y * i)) * weight[i];
            result += objectTexture.Sample(objectSamplerState, input.outTexCoord - float2(0.0, texOffset.y * i)) * weight[i];
        }
    }

    return float4(result, 1.0);
}
