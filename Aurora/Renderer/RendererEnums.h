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

enum Shader_Types
{
	// Vertex Shaders
	VS_Type_Object_Debug,
	VS_Type_Object_Common,
	VS_Type_Object_Simple,
	VS_Type_Sky,
	VS_Type_VertexColor,
	
	// Pixel Shaders
	PS_Type_Object,
	PS_Type_Object_Simple,
	PS_Type_Object_Debug,
	PS_Type_Sky_Static,
	PS_Type_Sky_Dynamic,
	PS_Type_PixelColor,

	Shader_Type_Count
};

enum Object_VertexInput
{
	InputSlot_PositionNormalWind,
	InputSlot_PreviousPosition,
	InputSlot_UV0,
	InputSlot_UV1,
	InputSlot_Atlas,
	InputSlot_Color,
	InputSlot_Tangent,
	InputSlot_InstanceData,
	InputSlot_Count
};

enum InputLayout_Types
{
	OnDemandTriangle,
	InputLayout_Position,
	InputLayout_Position_TexCoord,
	InputLayout_Position_PreviousPosition,
	InputLayout_Position_PreviousPosition_TexCoord,
	InputLayout_Common,
	InputLayout_Debug,
	InputLayout_RenderLightMap,
	InputLayout_VertexColor,
	InputLayout_Count,
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
	DS_Default,
	DS_Shadow,
	DS_XRay,
	DS_DepthRead,
	DS_DepthReadEqual,
	DS_EnvironmentalMap,
	DS_CaptureImposter,
	DS_WriteOnly,
	DS_Hologram,
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
	RS_Sky,
	RS_Count,
};

// Blend States
enum BS_Types
{
	BS_Opaque,
	BS_Transparent,
	BS_Inverse,
	BS_Additive,
	BS_Premultiplied,
	BS_ColorWriteDisable,
	BS_EnvironmentalLight,
	BS_Decal,
	BS_Multiply,
	BS_TransparentShadow,
	BS_Count
};

// Render Passes
enum RenderPass_Type
{
	RenderPass_Main
};

enum SkyRender_Type
{
	SkyRender_Static,
	SkyRender_Dynamic,
	SkyRender_Sun,
	SkyRender_Count
};

enum DebugRenderer_Type
{
	DebugRenderer_Grid,
	DebugRenderer_Count
};

// Textures
enum Texture_Types
{
	TextureType_2D_SkyAtmosphere_Transmittance_LUT,
	TextureType_2D_SkyAtmosphere_Multiscattered_Luminance_LUT,
	TextureType_2D_SkyAtmosphere_Sky_View_LUT,
	TextureType_2D_SkyAtmosphere_Sky_Luminance_LUT,
	TextureType_Count
};