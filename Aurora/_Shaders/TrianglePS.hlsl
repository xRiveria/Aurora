#include "Globals.hlsli"

TextureCube tex : register(t1);

// Outputs from vertex shader go here. Can be interpolated to pixel shader.
struct vs_out
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.

    float2 outTexCoord : TEXCOORD;
    float3 outNormal   : NORMAL;
    float3 outWorldSpace : WORLD_POSITION;
};

struct PS_Output
{
    float4 colorOutput : SV_Target0;
    float4 brightColorOutput : SV_Target1;
};

Texture2D objectTexture : TEXTURE: register(t0);
Texture2D objectTextureEmit : TEXTURE: register(t1);
SamplerState objectSamplerState : SAMPLER: register(s0);

// Attenuation = 1 / Constant + Linear Decrease (Distance) + Exponential (Distance Squared)

PS_Output main(vs_out input) : SV_TARGET // Pixel shader entry point which must return a float4 RGBA color value. 
{
    PS_Output psOutput;

    float3 norm = normalize(input.outNormal);
    float3 sampleColor = objectTexture.SampleLevel(objectSamplerState, input.outTexCoord, 0) * g_ObjectColor;
    // float3 ambientColor = 0.4f; // 0.4f ambient strength.
    // float3 lightDirection = normalize(-g_Light_Direction); // When using directional lights, position don't matter, just the direction.
    float3 emission = objectTextureEmit.SampleLevel(objectSamplerState, input.outTexCoord, 0);

    float3 finalColor = float3(0, 0, 0);
    for (int i = 0; i < 6; i++)
    {
        float3 vectorToLight = normalize(g_Light_Position[i] - input.outWorldSpace);
        float attenuationA = 1.0f;
        float attenuationB = 0.0f;
        float attenuationC = 0.1f;

        float distanceToLight = distance(g_Light_Position[i], input.outWorldSpace);
        float attenuationFactor = 1 / (attenuationA + attenuationB * distanceToLight + attenuationC * pow(distanceToLight, 2));
        float3 diff = max(dot(norm, vectorToLight), 0.0); // Ensure we are within 0 and 1. This is to ensure that we don't get a color darker than 0.0 if our light isn't near the object at all.
       
        float3 diffuse = diff * g_Light_Color[i] * attenuationFactor;
        // float3 ambientLight = g_Light_Color[i] * 1.0f;
        finalColor += diffuse;
        finalColor += emission;
    }

    // Emission

    float3 finalLight = finalColor * sampleColor;
  
    const float gammaCorrection = 2.2;

    // Reinhard Tone Mapping
    float3 toneMappedLighting = finalLight / (finalLight + float3(1.0, 1.0, 1.0));
    // Gamma Correction
    toneMappedLighting = pow(toneMappedLighting, float3(1.0, 1.0, 1.0) / gammaCorrection);

    psOutput.colorOutput = float4(toneMappedLighting, 1.0); // Must return an RGBA colour.
    
    // Check whether fragment output is higher than a threadshold, if so output into our bloom buffer.
    float brightness = dot(finalLight, float3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
    {
        psOutput.brightColorOutput = float4(psOutput.colorOutput);
    }
    else
    {
        psOutput.brightColorOutput = float4(0.0, 0.0, 0.0, 1.0);
    }

    return psOutput;
}