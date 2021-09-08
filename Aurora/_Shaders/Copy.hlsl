struct PixelInput_PositionUV
{
    float4 m_Position : SV_POSITION;
    float2 m_UV       : TEXCOORD;
};

SamplerState defaultSampler : SAMPLER: register(s3);
Texture2D gizmoTexture : register(t10);

float4 main(PixelInput_PositionUV PSIn) : SV_TARGET
{
    float4 sampleColor = gizmoTexture.SampleLevel(defaultSampler, PSIn.m_UV, 0).rgba;
    return float4(sampleColor);
}