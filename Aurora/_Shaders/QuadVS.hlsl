#include "Globals.hlsli"

struct VertexInput_PositionUV
{
    float3 m_Position : POSITION;
    float2 m_UV       : UV;
};

struct PixelInput_PositionUV
{
    float4 m_Position : SV_POSITION;
    float2 m_UV       : TEXCOORD;
};

PixelInput_PositionUV main(VertexInput_PositionUV inputData)
{
    PixelInput_PositionUV outputData;

    float4 position = float4(inputData.m_Position, 1.0f);
    outputData.m_Position = mul(g_OrthographicViewProjection, position);
    outputData.m_UV = inputData.m_UV;

    return outputData;
}