#pragma once

enum BlendMode_Types
{
	BlendMode_Opaque,
	BlendMode_Alpha,
	BlendMode_Premultiplied,
	BlendMode_Additive,
	BlendMode_Multiply,
	BlendMode_Count
};

enum InputLayout_Types
{
	OnDemandTriangle,
	InputLayout_Position,
	InputLayout_Position_Texture,
	InputLayout_Position_PreviousPosition,
	InputLayout_Position_PreviousPosition_Texture,
	InputLayout_Common,
	InputLayout_Debug,
	InputLayout_RenderLightMap,
	InputLayout_VertexColor,
	InputLayout_Count
};

enum GBuffer_Types
{
	GBuffer_Color,
	GBuffer_Normal_Roughness,
	GBuffer_Velocity,					// Motion Blur
	GBuffer_Count
};

// Constant Buffers
enum CB_Types
{
	CB_Frame,
	CB_Camera,
	CB_Misc,
	CB_Count
};

// Depth Stencil States
enum DS_Types
{
	DS_Opaque,
	DS_Transparent,
	DS_Inverse,
	DS_Additive,
	DS_Premultiplied,
	DS_ColorWriteDisabled,
	DS_EnvironmentalLight,
	DS_Decal,
	DS_Multiply,
	DS_TransparentShadow,
	DS_Count
};

// Rasterizer States
enum RS_Types
{
	RS_Front,
	RS_Back,
	RS_DoubleSided,
	RS_Wire,
	RS_Wire_Smooth,
	RS_Wire_DoubleSided,
	RS_Wire_DoubleSided_Smooth,
	RS_Shadow,
	RS_Shadow_DoubleSided,
	RS_Occlude,
	RS_Voxelize,
	RS_Sky,
	RS_Count
};

// Render Passes
enum RenderPass_Type
{
	RenderPass_Main
};