#include "Globals.hlsli"

struct VSOut
{
    float4 position : SV_POSITION;
    float2 clipSpace : TEXCOORD;
};

VSOut main(uint vI : SV_VERTEXID)
{
    VSOut output;
    FullScreenTriangle(vI, output.position);

    output.clipSpace = output.position.xy;

    return output;
}