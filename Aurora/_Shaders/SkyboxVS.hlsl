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
    float3 localPosition : TEXCOORD1;
};

InputPixelType main(InputVertexType input)
{
    InputPixelType output;
    output.localPosition = input.inPosition.xyz;

    float4x4 newView = g_Camera_View;
    newView[3][0] = 0.0;
    newView[3][1] = 0.0;
    newView[3][2] = 0.0;

    float4 worldPosition = float4(input.inPosition, 1.0);
    output.outPosition = mul(worldPosition, newView);
    output.outPosition = mul(output.outPosition, g_Camera_Projection);

    output.outPosition = output.outPosition.xyzw;
    output.outPosition.z = output.outPosition.w * 0.9999;

    return output;
}