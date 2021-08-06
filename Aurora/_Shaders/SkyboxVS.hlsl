// #pragma pack_matrix ( row_major )

#include "Globals.hlsli"

struct VertexInput
{
    float3 inPosition : POSITION;  // Triangle made from 3 XYZ positions.
};

struct PixelShaderInput
{
    float3 localPosition : POSITION;
    float4 pixelPosition : SV_POSITION;
};

PixelShaderInput main(VertexInput input)
{
    PixelShaderInput output;
    output.localPosition = input.inPosition.xyz;

    float4x4 newView = g_Camera_View;
    newView[3][0] = 0.0;
    newView[3][1] = 0.0;
    newView[3][2] = 0.0;

    float4 worldPosition = float4(input.inPosition, 1.0);
     output.pixelPosition = mul(worldPosition, newView);
   // output.pixelPosition = mul(worldPosition, g_Camera_View);
    output.pixelPosition = mul(output.pixelPosition, g_Camera_Projection);

   output.pixelPosition = output.pixelPosition.xyzw;
   output.pixelPosition.z = output.pixelPosition.w * 0.9999;

    return output;
}