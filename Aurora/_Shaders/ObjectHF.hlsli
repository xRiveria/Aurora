cbuffer objectConstantBuffer : register(b0)
{
    float4x4 mvp;
    float4x4 worldMatrix;
}

#ifdef OBJECTSHADER_LAYOUT_POS_TEX // Used by transparent objects or shadows with alpha testing.

#define OBJECTSHADER_INPUT_POS
#define OBJECTSHADER_INPUT_TEX
#define OBJECTSHADER_USE_WIND
#define OBJECTSHADER_USE_UVSETS
#define OBJECTSHADER_USE_COLOR

#endif // OBJECTSHADER_LAYOUT_POS_TEX

// Information from Input Layout:
struct VertexInput
{
    float4 position : POSITION_NORMAL_WIND;

    float4 GetPosition()
    {
        return float4(position.xyz, 1);
    }

    float3 GetNormal()
    {
        return float3(0, 0, 0);
    }

    float GetWindWeight()
    {
        return 0.0;
    }

#ifdef OBJECTSHADER_INPUT_TEX
    float2 uv0 : UVSET0;
    float2 uv1 : UVSET1;

    float2 GetUV0()
    {
        return uv0;
    }

    float2 GetUV1()
    {
        return uv1;
    }
#endif // OBJECTSHADER_INPUT_TEX
};

struct PixelInput
{
    precise float4 position : SV_POSITION;

#ifdef OBJECTSHADER_USE_COLOR
    float4 color : COLOR;
#endif // OBJECTSHADER_USE_COLOR

#ifdef OBJECTSHADER_USE_UVSETS
    float4 uvsets : UVSETS;
#endif
};

struct VertexSurface
{
    float4 position;
    float4 uvsets;
    float4 color;

    inline void Create(in VertexInput input)
    {
        // float4x4 WOLRD = float4x4
        position = input.GetPosition();
        color = float4(1, 1, 1, 1);

#ifdef OBJECTSHADER_INPUT_TEX
        uvsets = float4(input.GetUV0(), input.GetUV1());
#endif

        position = mul(mvp, position);
    }
};

#ifdef OBJECTSHADER_COMPILE_VS

// Vertex Shader Base
PixelInput main(VertexInput input)
{
    PixelInput output;

    VertexSurface surface;
    surface.Create(input);

    output.position = mul(input.GetPosition(), mvp);

#ifdef OBJECTSHADER_USE_COLOR
    output.color = surface.color;
#endif // OBJECTSHADER_USE_COLOR

#ifdef OBJECTSHADER_USE_UVSETS
    output.uvsets = surface.uvsets;
#endif // OBJECTSHADER_USE_UVSETS

    return output;
}

#endif // OBJECTSHADER_COMPILE_VS