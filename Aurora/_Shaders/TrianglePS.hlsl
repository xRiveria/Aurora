// Outputs from vertex shader go here. Can be interpolated to pixel shader.
struct vs_out
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.
    float2 outTexCoord : TEXCOORD;
    float3 outNormal   : NORMAL;
};

Texture2D objectTexture : TEXTURE: register(t0);
SamplerState objectSamplerState : SAMPLER: register(s0);

cbuffer lightConstantBuffer : register(b0)
{
    float3 ambientLightColor;
    float ambientLightStrength;
}

float4 main(vs_out input) : SV_TARGET // Pixel shader entry point which must return a float4 RGBA color value. 
{
    float3 sampleColor = objectTexture.Sample(objectSamplerState, input.outTexCoord);
    float3 ambientLight = ambientLightColor * ambientLightStrength;
    float3 finalColor = sampleColor * ambientLight;

    return float4(finalColor, 1.0); // Must return an RGBA colour.
}