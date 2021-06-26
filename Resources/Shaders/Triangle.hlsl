// Vertex attributes go here as input to the vertex shader.
struct vs_in
{
    float3 inPosition : POS;  // Triangle made from 3 XYZ positions.
    float2 inTexCoord : TEXCOORD;
};

// Outputs from vertex shader go here. Can be interpolated to pixel shader.
struct vs_out
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.
    float2 outTexCoord : TEXCOORD;
};

vs_out vs_main(vs_in input)  // Vertex shader entry point called vs_main(). Entry points and structs can be named whatever we like. 
{
    vs_out output = (vs_out)0; // Zero the memory first.
    output.outPosition = float4(input.inPosition, 1.0); // Vertex shader must output a float4 XYZW value to set the homogenous clip space (betwen -1 and 1 in XY axis and 0 and 1 in Z axis.
    output.outTexCoord = input.inTexCoord;
    return output;
}

Texture2D objectTexture : TEXTURE : register(t0);
SamplerState objectSamplerState : SAMPLER: register(s0);

float4 ps_main(vs_out input) : SV_TARGET // Pixel shader entry point which must return a float4 RGBA color value. 
{
    float3 pixelColor = objectTexture.Sample(objectSamplerState, input.outTexCoord);
    return float4(pixelColor, 1.0); // Must return an RGBA colour.
}