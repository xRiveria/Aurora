#include "Globals.hlsli"

struct VertexInput_PositionUV
{
    float4 m_Position : POSITION0;
    float2 m_UV       : TEXCOORD0;
};

struct PixelInput_PositionUV
{
    float4 m_Position : SV_POSITION;
    float2 m_UV       : TEXCOORD;
};

PixelInput_PositionUV main(VertexInput_PositionUV inputData)
{
    PixelInput_PositionUV outputData;

    inputData.m_Position.w = 1.0f;
    outputData.m_Position = mul(inputData.m_Position, g_ViewProjection_Orthographic);
    outputData.m_UV = inputData.m_UV;

    return outputData;
}