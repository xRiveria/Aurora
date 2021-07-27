#include "ShaderUtilities.h"

/* Personal Notes

    We are utilizing a slot based binding model. However, it does have a CPU overhead. Avoid binding massive amount of resources. The maximum slot numbers that should be used are:
    
    - 15 for BindConstantBuffer(),
    - 64 for BindResource(),
    - 8  for BindUAV()
*/

static const uint Entity_Type_DirectionalLight = 0;
static const uint Entity_Type_PointLight = 1;
static const uint Entity_Type_SpotLight = 2;

struct ShaderMaterialData
{
    float4 g_ObjectColor;

    float g_Roughness;
    float g_Metalness;
    float g_Padding001;
    float g_Padding002;
};

// On Demand Constant Buffers

CBUFFER(ConstantBufferData_Material, CBSLOT_RENDERER_MATERIAL)
{
    ShaderMaterialData g_Material;

    float3 g_Camera_Position;
    int g_Texture_SkyHDRMap_Index;

    int g_Texture_BaseColorMap_Index;
    int g_Texture_NormalMap_Index;
    int g_Texture_MetalnessMap_Index;
    int g_Texture_RoughnessMap_Index;

    int g_Texture_TextureCube_Index;
    int padding00001;
    int padding00002;
    int padding00003;

    float4x4 g_ObjectMatrix; // Temporary
    float4x4 g_WorldMatrix;
};

CBUFFER(ConstantBufferData_Misc, CBSLOT_RENDERER_MISC)
{
    float4x4 g_Transform;
    float4   g_Color;

    float    g_IsHorizontalPass;
    float3   g_Light_Direction;
};

// Common Constant Buffers
CBUFFER(ConstantBufferData_Frame, CBSLOT_RENDERER_FRAME)
{
    float4   g_Light_Position[4];
    float4   g_Light_Color[4];
};

// 32 bit alignment.
CBUFFER(ConstantBufferData_Camera, CBSLOT_RENDERER_CAMERA)
{
    float4x4 g_Camera_ViewProjection;
    float4x4 g_Camera_InverseViewProjection;

    float4x4 g_Camera_View;
    float4x4 g_Camera_Projection;

    float g_RenderItem; // 0 skybox.
    float g_Padding1;
    float g_Padding2;
    float g_Padding3;
};