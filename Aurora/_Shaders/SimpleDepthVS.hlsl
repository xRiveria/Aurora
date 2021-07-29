#pragma pack_matrix ( row_major )

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
    InputPixelType output = (InputPixelType)0; // Zero the memory first.

    output.outPosition = mul(float4(input.inPosition, 1.0), g_ModelMatrix); // Vertex shader must output a float4 XYZW value to set the homogenous clip space (betwen -1 and 1 in XY axis and 0 and 1 in Z axis.    
    output.outPosition = mul(output.outPosition, g_LightSpaceMatrix);

    return output;


}