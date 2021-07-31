#include "Globals.hlsli"
#include "Constants.hlsli"
#include "Mappings.hlsli"

// Outputs from vertex shader go here. Can be interpolated to pixel shader.
struct vs_out
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.

    float2 outTexCoord : TEXCOORD;
    float3 outNormal   : NORMAL;
    float3 outWorldSpace : WORLD_POSITION;
    float4 outFragPosInLightSpace : LIGHT_POSITION;
};

struct PS_Output
{
    float4 colorOutput : SV_Target0;
    float4 brightColorOutput : SV_Target1;
};

Texture2D bloomBlur : TEXTURE: register(t1);
SamplerState objectSamplerState : SAMPLER: register(s0);
SamplerState objectSamplerState2 : SAMPLER: register(s1);

float ShadowCalculation(float4 fragPositionInLightSpace)
{
    // Transform the light space fragment position in clip space to NDC. 
    float3 projectionCoords = fragPositionInLightSpace.xyz / fragPositionInLightSpace.w; // Returns position in the range -1 and 1.
    // projectionCoords = projectionCoords * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f); // As the depth from the depth map is in the range [0, 1], we transfrom our NDC to the range [0, 1] for sampling from the depth map.
    float closestDepth = Texture_ShadowDepthMap.SampleLevel(objectSamplerState2, projectionCoords.xy, 0).r; // Gives us the closest depth from the light's POV.
    float currentDepth = projectionCoords.z - g_Light_Bias; // Retrieve the current fragment's depth.
    float isInShadow = currentDepth > closestDepth ? 1.0 : 0.0;

    if (projectionCoords.z > 1.0)
    {
        isInShadow = 0.0;
    }
    return isInShadow;

    // return isInShadow;
}

// Attenuation = 1 / Constant + Linear Decrease (Distance) + Exponential (Distance Squared)

PS_Output main(vs_out input) : SV_TARGET // Pixel shader entry point which must return a float4 RGBA color value. 
{
    PS_Output psOutput;

    // ======
    float3 pixelWorldPosition = input.outWorldSpace;
    float3 viewDirection = normalize(g_Camera_Position.xyz - pixelWorldPosition);
    
    float3 normal = Texture_NormalMap.SampleLevel(objectSamplerState, input.outTexCoord, 0).rgb * input.outNormal;
    float3 albedoColor = pow(Texture_BaseColorMap.SampleLevel(objectSamplerState, input.outTexCoord, 0).rgb, 2.2) * g_Material.g_ObjectColor; // Reconvert to linear space.
    float roughness = Texture_RoughnessMap.SampleLevel(objectSamplerState, input.outTexCoord, 0).r * g_Material.g_Roughness;
    float metalness = Texture_MetalnessMap.SampleLevel(objectSamplerState, input.outTexCoord, 0).r * g_Material.g_Metalness;
    
    float3 normalVector = normalize(normal);
    // Reflectance Equation
    float3 F0 = float3(0.04, 0.04, 0.04); // Precompute Freshnel for both dieletrics and conductors so the same Fresnel-Schlick approximation can be used for both. Read!
    F0 = lerp(F0, albedoColor, metalness);
    
    float3 Lo = float3(0.01f, 0.01f, 0.01f); // Total irradiance generated by our light sources.

    float shadowFactor = 1.0 - ShadowCalculation(input.outFragPosInLightSpace);

    for (int i = 0; i < 4; i++)
    {
        // Calculate per-light radiance.
        float3 lightDirection = normalize(g_Light_Position[i].xyz - pixelWorldPosition);
        float3 halfwayVector = normalize(viewDirection + lightDirection);
        float distance = length(g_Light_Position[i].xyz - pixelWorldPosition);
        float attenuation = 1.0 / (distance * distance); 
        float3 radiance = (g_Light_Color[i].xyz * g_Light_Color[i].w * 5.0) * attenuation;

        // Cook-Torrace BRDF
        float NDF = DistributionGGX(normalVector, halfwayVector, roughness);
        float G = GeometrySmith(normalVector, viewDirection, lightDirection, roughness);
        float3 F = FresnelSchlick(max(dot(halfwayVector, viewDirection), 0.0), F0);

        // Energy Conservation
        float3 kSpecular = F; // The amount of light that gets reflected.
        float3 kDiffuse = float3(1.0, 1.0, 1.0) - kSpecular; // The remaining light energy that gets refracted.
        kDiffuse *= 1.0 - metalness; // As metallic surfaces don't refract light and have no diffuse reflections, we enforce this by nullying kDiffuse if the surface is metallic. 

        // Final BRDF Computation
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normalVector, viewDirection), 0.0) * max(dot(normalVector, lightDirection), 0.0);
        float3 specular = numerator / max(denominator, 0.001); // We clamp this to 0.001 to prevent any division by zero.

        float NDotL = max(dot(normalVector, lightDirection), 0.0); // Scale light by NDotL.

        // Add to outgoing radiance Lo.
        Lo += (kDiffuse * albedoColor * shadowFactor / PI + specular) * radiance * NDotL; // Reflectance Equation.
    }
     
    // Lo *= float3(inverseShadow, inverseShadow, inverseShadow);
    float3 finalColor = float3(0.15, 0.15, 0.15) * Lo; // Our ambient is currently a constant factor. For IBL, we will take this into account.
    
    // Reinhard Tone Mapping
    const float gammaCorrectionFactor = 2.2;
    finalColor = finalColor / (finalColor + float3(1.0, 1.0, 1.0));
    finalColor = pow(finalColor, float3(1.0 / gammaCorrectionFactor, 1.0 / gammaCorrectionFactor, 1.0 / gammaCorrectionFactor));
    
    // Return to main.
    psOutput.colorOutput = float4(finalColor, 1.0);
    
    // Check whether fragment output is higher than a threadshold, if so output into our bloom buffer.
    float brightness = dot(finalColor, float3(0.2126, 0.7152, 0.0722));
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