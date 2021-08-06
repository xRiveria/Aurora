static const float PI = 3.14159265359;

// Injects a roughness parameter for ambient lighting.
// We earlier use a halfway vector to determine the Fresnel response, but it doesn't work for IBL (hemisphere) as there's no single halfway vector avaliable.
// Without it, the surface's reflective ratio will always end up high. It shouldn't be as indirect light follows the properties of direct light and hence, rougher surfaces should reflect less strongly.
// Hence, without it, the Fresnel reflection strength looks off on non-metal surfaces. We will inject a roughness parameter to allievate the issue.
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// Calculates how much the surface reflects light versus how much it refracts light, varying over the angle we're looking at the surface.
// We make an assumption that dieletrics have a constant F0 of 0.04.
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// Approximates the relative surface area of microfacets exactly aligned to the halfway vector. The more the alignment, the surface is smooth, hence a very bright spot is shown.
// Otherwise, the microfacets are aligned in much more random directions, giving a less concentrated effect, giving us more of a spread across the entire object.
float DistributionGGX(float3 normalVector, float3 halfwayVector, float roughness)
{
    float a = roughness * roughness; // Based on observations by Disney/Epic Games, the lighting looks more correct squaring the roughness in both the geometry and normal distributive functions.
    float a2 = a * a;
    float NDotH = max(dot(normalVector, halfwayVector), 0.0);
    float NDotH2 = NDotH * NDotH;

    float numerator = a2;
    float denominator = (NDotH2 * (a2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return numerator / max(denominator, 0.000001); // Prevent divide by zero for Roughness = 0.0 and NDotH = 1.0.
}

// Statistically approximates the relative surface area where its micro surface-details overshadow others, causing light rays to be occluded. Rougher surfaces have a higher probability of overshadowing microfacets.
// Takes into account the view direction (geometry obstruction) and the light direction vector (geometry shadowing).
float GeometrySchlickGGX(float NDotV, float roughness)
{
    // Direct Lighting and IBL Lighting have different formulas for the roughness parameter. For this, we will be using Direct Lighting.
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float numerator = NDotV;
    float denominator = NDotV * (1.0 - k) + k;

    return numerator / denominator;
}

float GeometrySmith(float3 normalVector, float3 viewDirection, float3 lightDirection, float roughness)
{
    float NDotV = max(dot(normalVector, viewDirection), 0.0);
    float NDotL = max(dot(normalVector, lightDirection), 0.0);

    float ggx1 = GeometrySchlickGGX(NDotL, roughness);
    float ggx2 = GeometrySchlickGGX(NDotV, roughness);

    return ggx1 * ggx2;
}