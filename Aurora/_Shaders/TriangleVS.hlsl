#pragma pack_matrix ( row_major )
#include "Globals.hlsli"

cbuffer objectConstantBuffer : register(b0)
{
    float4x4 mvp;
    float4x4 worldMatrix;
}

// Vertex attributes go here as input to the vertex shader.
struct vs_in
{
    float3 inPosition : POS;  // Triangle made from 3 XYZ positions.
    float2 inTexCoord : TEXCOORD;
    float3 inNormal   : NORMAL;
};

// Outputs from vertex shader go here. Can be interpolated to pixel shader.
struct vs_out
{
    float4 outPosition : SV_POSITION; // The position is identified by the SV_POSITION semantic.
    float2 outTexCoord : TEXCOORD;
    float3 outNormal   : NORMAL;
};

vs_out main(vs_in input)  // Vertex shader entry point called vs_main(). Entry points and structs can be named whatever we like. 
{
    vs_out output = (vs_out)0; // Zero the memory first.
    output.outPosition = mul(float4(input.inPosition, 1.0), g_Camera_ViewProjection); // Vertex shader must output a float4 XYZW value to set the homogenous clip space (betwen -1 and 1 in XY axis and 0 and 1 in Z axis.
    output.outTexCoord = input.inTexCoord;
    output.outNormal = normalize(mul(float4(input.inNormal, 0.0), worldMatrix)); // Length of 1 Normals
    return output;
}