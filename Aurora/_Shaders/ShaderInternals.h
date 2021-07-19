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

// On Demand Constant Buffers

CBUFFER(ConstantBufferData_Misc, CBSLOT_RENDERER_MISC)
{
    float4x4 g_Transform;
    float4   g_Color;

    // Temporary
    float4   g_Light_Position[6];
    float4   g_Light_Color[6];

    float3   g_Light_Direction;  
    float roughness;
};

// Common Constant Buffers
CBUFFER(ConstantBufferData_Frame, CBSLOT_RENDERER_FRAME)
{

};

// 32 bit alignment.
CBUFFER(ConstantBufferData_Camera, CBSLOT_RENDERER_CAMERA)
{
    float4x4 g_ObjectMatrix; // Temporary
    float4x4 g_WorldMatrix;

    float4x4 g_Camera_ViewProjection;
    float4x4 g_Camera_InverseViewProjection;

    float3 g_Camera_Position;
    uint g_Padding;

    float4x4 g_Camera_View;
    float4x4 g_Camera_Projection;

    float4 g_ObjectColor;

    float g_RenderItem; // 0 skybox.
    float g_Padding1;
    float g_Padding2;
    float g_Padding3;
};