#include "Globals.hlsli"

inline ShaderMaterial GetMaterial()
{
    return g_Material;
}

// These are bound by the Renderer (based on Material).
TEXTURE2D(texture_BaseColorMap, float4, TEXSLOT_RENDERER_BASECOLOR_MAP);         // RGB: Base Color, A: Opacity
TEXTURE2D(texture_NormalMap, float3, TEXSLOT_RENDERER_NORMAL_MAP);              // RGB: Normal
TEXTURE2D(texture_SurfaceMap, float4, TEXSLOT_RENDERER_SURFACE_MAP);            // R: Occlusion, G: Roughness, B: Metallic, A: Reflectance
TEXTURE2D(texture_EmissiveMap, float4, TEXSLOT_RENDERER_EMISSIVE_MAP);          // RGBA: Emissive
TEXTURE2D(texture_DisplacementMap, float, TEXSLOT_RENDERER_DISPLACEMENT_MAP);   // R: Height Map
TEXTURE2D(texture_OcclusionMap, float, TEXSLOT_RENDERER_OCCLUSION_MAP);         // R: Occlusion
TEXTURE2D(texture_TransmissionMap, float, TEXSLOT_RENDERER_TRANSMISSION_MAP);   // R: Transmission Factor
TEXTURE2D(texture_SpecularMap, float4, TEXSLOT_RENDERER_SPECULAR_MAP);          // RGB: Color, A: Intensity

// Used by transparent objects or shadows with alpha testing.
#ifdef OBJECTSHADER_LAYOUT_POS_TEX

#define OBJECTSHADER_INPUT_POS
#define OBJECTSHADER_INPUT_TEX
#define OBJECTSHADER_USE_WIND
#define OBJECTSHADER_USE_UVSETS
#define OBJECTSHADER_USE_COLOR

#endif // OBJECTSHADER_LAYOUT_POS_TEX

// Used by common render passes.
#ifdef OBJECTSHADER_LAYOUT_COMMON

#define OBJECTSHADER_INPUT_POS
#define OBJECTSHADER_INPUT_TEX
#define OBJECTSHADER_INPUT_COL
#define OBJECTSHADER_INPUT_TAN
// #define OBJECTSHADER_USE_CLIPPLANE
#define OBJECTSHADER_USE_WIND
#define OBJECTSHADER_USE_UVSETS
#define OBJECTSHADER_USE_COLOR
#define OBJECTSHADER_USE_NORMAL
#define OBJECTSHADER_USE_TANGENT
#define OBJECTSHADER_USE_POSITION3D
#define OBJECTSHADER_USE_EMISSIVE

#endif // OBJECTSHADER_LAYOUT_COMMON

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
        const uint normalWind = asuint(position.w); // Return as unsigned integer. Remember that we store the normal and wind information in the position float through bit shifting.
        float3 normal;

        normal.x = (float)((normalWind >> 0u) & 0xFF)  / 255.0 * 2 - 1;
        normal.y = (float)((normalWind >> 8u) & 0xFF)  / 255.0 * 2 - 1;
        normal.z = (float)((normalWind >> 16u) & 0xFF) / 255.0 * 2 - 1;

        return normal;
    }

    float GetWindWeight()
    {
        const uint normalWind = asuint(position.w);
        return ((normalWind >> 24u) & 0xFF) / 255.0;
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

#ifdef OBJECTSHADER_INPUT_COL
    float4 color : COLOR;

    float4 GetVertexColor()
    {
        return color;
    }
#endif

#ifdef OBJECTSHADER_INPUT_TAN
    float4 tangent : TANGENT;

    float4 GetTangent()
    {
        return tangent * 2 - 1;
    }
#endif
};

struct PixelInput
{
    precise float4 position : SV_POSITION;

#ifdef OBJECTSHADER_USE_COLOR
    float4 color : COLOR;
#endif

#ifdef OBJECTSHADER_USE_UVSETS
    float4 uvsets : UVSETS;
#endif

#ifdef OBJECTSHADER_USE_NORMAL
    float3 normal : NORMAL;
#endif

#ifdef OBJECTSHADER_USE_TANGENT
    float3 tangent : TANGENT;
#endif

#ifdef OBJECTSHADER_USE_POSITION3D
    float3 position3D : WORLDPOSITION;
#endif

#ifdef OBJECTSHADER_USE_EMISSIVE
    uint emissiveColor : EMISSIVECOLOR;
#endif
};

struct VertexSurface
{
    float4 position;
    float4 uvsets;
    float4 color;
    float3 normal;
    float4 tangent;
    uint emissiveColor;

    inline void Create(in ShaderMaterial material, in VertexInput input)
    {
        float4x4 worldMatrix = g_ObjectWorldMatrix; // Object's transform in world space.
        position = input.GetPosition();
        color = material.material_BaseColor;
        emissiveColor = material.material_EmissiveColor;

#ifdef OBJECTSHADER_INPUT_TEX
        uvsets = float4(input.GetUV0(), input.GetUV1()); // Once we have texture displacement, we will multiply our current UVs with the displacement information.
#endif

#ifdef OBJECTSHADER_INPUT_COL
        if (material.IsUsingVertexColors())
        {
            color *= input.GetVertexColor();
        }
#endif

#ifdef OBJECTSHADER_INPUT_TAN
        tangent = input.GetTangent();
        tangent.xyz = normalize(mul((float3x3)worldMatrix, tangent.xyz));
#endif

#ifdef OBJECTSHADER_USE_WIND
        // No wind support at the moment. In short, we will use wind information to displace our position.
#endif
        normal = normalize(mul((float3x3)worldMatrix, input.GetNormal())); // Transform normals to world coordinates.
        float4x4 mvp = worldMatrix * g_Camera_ViewProjection;
        position = mul(mvp, position);
    }
};

// Object Shader Prototype
#ifdef OBJECTSHADER_COMPILE_VS

// Vertex Shader Base
PixelInput main(VertexInput input)
{
    PixelInput output;

    VertexSurface surface;
    surface.Create(GetMaterial(), input);

    output.position = surface.position;

#ifdef OBJECTSHADER_USE_POSITION3D
    output.position3D = surface.position.xyz;
#endif

#ifdef OBJECTSHADER_USE_COLOR
    output.color = surface.color;
#endif // OBJECTSHADER_USE_COLOR

#ifdef OBJECTSHADER_USE_UVSETS
    output.uvsets = surface.uvsets;
#endif // OBJECTSHADER_USE_UVSETS

#ifdef OBJECTSHADER_USE_NORMAL
    output.normal = surface.normal;
#endif

#ifdef OBJECTSHADER_USE_TANGENT
    output.tangent = surface.tangent;
#endif

#ifdef OBJECTSHADER_USE_EMISSIVE
    output.emissiveColor = surface.emissiveColor;
#endif

    return output;
}

#endif // OBJECTSHADER_COMPILE_VS




#ifdef OBJECTSHADER_COMPILE_PS
float4 main(PixelInput input, in bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
    // const float depth = input.position.z;
    // const float linearDepth = input.position.w;
    // const float2 pixel = input.position.xy;
    // Screen Coord.
    // float3 bumpColor = 0;
    return float4(1, 1, 1, 1);
}
#endif