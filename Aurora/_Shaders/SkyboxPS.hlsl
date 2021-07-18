TextureCube tex : register(t1);
SamplerState samp : register(s1);

float4 main(float3 worldPosition : POSITION) : SV_TARGET
{
    return tex.Sample(samp, worldPosition);
}