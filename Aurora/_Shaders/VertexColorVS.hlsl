#include "Globals.hlsli"

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexToPixel main(float4 inPos : POSITION, float4 inColor : TEXCOORD0)
{
    VertexToPixel Out;
    
    Out.position = mul(g_Transform, inPos);
    Out.color = inColor * g_Color;
    
    return Out;
}