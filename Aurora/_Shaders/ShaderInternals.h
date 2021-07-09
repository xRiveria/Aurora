#ifdef __cplusplus
#pragma once
#endif
#include "ShaderUtilities.h"

/* Personal Notes

    We are utilizing a slot based binding model. However, it does have a CPU overhead. Avoid binding massive amount of resources. The maximum slot numbers that should be used are:
    
    - 15 for BindConstantBuffer(),
    - 64 for BindResource(),
    - 8  for BindUAV()
*/

struct ShaderMesh
{
    int m_Index_Buffer;
    int m_Vertex_Buffer_Position;
    int m_Vertex_Buffer_Tangent;
    int m_Vertex_Buffer_Color;

    int m_Vertex_Buffer_UV0;
    int m_Vertex_Buffer_UV1;
};

struct ShaderMaterial
{
    float4 material_BaseColor;
    float4 material_SpecularColor;
    float4 material_EmissiveColor;

    float material_Roughness;
    float material_Metalness;
    float material_NormalMapStrength;
    float material_DisplacementMapping;

    int material_UVSet_BaseColorMap;
    int material_UVSet_SurfaceMap;
    int material_UVSet_NormalMap;
    int material_UVSet_DisplacementMap;

    int material_UVSet_EmissiveMap;
    int material_UVSet_OcclusionMap;
    int material_UVSet_TransmissionMap;
    int material_UVSet_SpecularMap;

    //=====================================

    int texture_BaseColorMap_Index;
    int texture_SurfaceMap_Index;
    int texture_NormalMap_Index;
    int texture_DisplacementMap_Index;

    int texture_EmissiveMap_Index;
    int texture_OcclusionMap_Index;
    int texture_TransmissionMap_Index;
    int texture_SpecularMap_Index;
};

static const uint Entity_Type_DirectionalLight = 0;
static const uint Entity_Type_PointLight = 1;
static const uint Entity_Type_SpotLight = 2;

// On Demand Constant Buffers

CBUFFER(ConstantBufferData_Misc, CBSLOT_RENDERER_MISC)
{
    float4x4 g_Transform;
    float4   g_Color;

    // Temporary
    float4   g_Light_Position;
    float4   g_Light_Color;
};

CBUFFER(ConstantBufferData_Material, CBSLOT_RENDERER_MATERIAL)
{

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
};