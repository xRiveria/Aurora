#include "Globals.hlsli"

struct VSOut
{
    float3 worldPosition : POSITION;
    float4 position : SV_POSITION;
};

VSOut main(float3 position : POS, float2 inTexCoord : TEXCOORD, float3 inNormal : NORMAL)
{
    VSOut output = (VSOut)0;
    output.worldPosition = position;
    output.position = mul(float4(position, 1.0), g_ObjectMatrix);
    // output.position.z = output.position.w; 

    return output;
}