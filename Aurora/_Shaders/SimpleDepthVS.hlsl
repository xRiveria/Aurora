#include "Globals.hlsli"

struct InputVertexType
{
    float3 inPosition : POS;  // Triangle made from 3 XYZ positions.
    float2 inTexCoord : TEXCOORD;
    float3 inNormal   : NORMAL;
};

struct InputPixelType
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.
};

InputPixelType main(InputVertexType input)
{
    InputPixelType output;
   
    output.outPosition = mul(float4(input.inPosition, 1.0f), g_WorldMatrix);
    output.outPosition = mul(output.outPosition, g_LightSpaceMatrix);

    return output;
}