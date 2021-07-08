
float3 CustomAtmosphericScattering(float3 V)
{
    const float3 skyColor = float3(0.42f, 0.42f, 0.42f);   // Zenith Color

    const float zenith = V.y;  // How much is above. 0: Horizon, 1: Directly above.

    return skyColor;
}

// Returns a sky color modulated by the sun and clouds.
float3 GetDynamicSkyColor(in float3 view, bool cloudsEnabled = true)
{
    float3 sky = float3(0, 0, 0);

    sky = CustomAtmosphericScattering(view);

    return sky;
}
