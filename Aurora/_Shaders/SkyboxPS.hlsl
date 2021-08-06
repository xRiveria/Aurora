TextureCube environmentTexture : register(t5);
SamplerState defaultSampler : register(s3);

struct PixelShaderInput
{
    float3 localPosition : POSITION;
    float4 pixelPosition : SV_POSITION;
};

float4 main(PixelShaderInput input) : SV_Target
{
    float3 environmentVector = normalize(input.localPosition);
    return environmentTexture.SampleLevel(defaultSampler, environmentVector, 1);
    // return float4(1, 1, 1, 1);
}