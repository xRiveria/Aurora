#pragma once
#include "Renderer.h"
#include "FileSystem.h"
#include "../Graphics/DX11/DX11_Utilities.h"
#include "../Scene/World.h"
#include "../Scene/Components/MeshUtilities.h"

namespace Aurora
{
    namespace RendererGlobals
    {
        RHI_Shader            g_Shaders[Shader_Types::Shader_Type_Count];
        RHI_DepthStencilState g_DepthStencilStates[DS_Types::DS_Count];
        RHI_BlendState        g_BlendStates[BS_Types::BS_Count];
        RHI_RasterizerState   g_RasterizerStates[RS_Types::RS_Count];
        RHI_GPU_Buffer        g_ConstantBuffers[CB_Types::CB_Count];
        RHI_InputLayout       g_InputLayouts[InputLayout_Types::InputLayout_Count];
        RHI_Texture           g_Textures[Texture_Types::TextureType_Count];

        // Pipeline States
        RHI_PipelineState m_PSO_Object_Wire; // Right now we're using this for everything.
        RHI_PipelineState m_PSO_Object_Sky[SkyRender_Count];
        RHI_PipelineState m_PSO_Object_Debug[DebugRenderer_Count];
        RHI_PipelineState m_PSO_Object; /// Standard 1 for now for all objects. Naturally, we will need more for different types of objects.

        std::string g_ShaderPath = "_Shaders/";
        std::string g_ShaderSourcePath = "../Aurora/_Shaders/";
    }

    // We would load in a binary format usually. For now, we still stick to our default HLSL files and pass it to our compiler.
    bool Renderer::LoadShader(Shader_Stage shaderStage, RHI_Shader& shader, const std::string& fileName, Shader_Model minimumShaderModel)
    {
        std::string shaderFileName = RendererGlobals::g_ShaderPath + fileName;
        m_ShaderCompiler.RegisterShader(shaderFileName);

        if (m_ShaderCompiler.IsShaderOutdated(shaderFileName))
        {
            ShaderCompiler::CompilerInput input;
            input.m_Format = m_GraphicsDevice->GetShaderFormat();   // HLSL5, HLSL6 or SPIRV.
            input.m_Stage = shaderStage;
            input.m_MinimumShaderModel = minimumShaderModel;

            std::string sourceDirectory = FileSystem::MakePathAbsolute(RendererGlobals::g_ShaderSourcePath);
            input.m_IncludeDirectories.push_back(sourceDirectory);
            input.m_ShaderSourceFilePath = FileSystem::ReplaceExtension(sourceDirectory + fileName, "hlsl");

            ShaderCompiler::CompilerOutput output;
            m_ShaderCompiler.Compile(input, output);

            if (output.IsValid())
            {
                m_ShaderCompiler.SaveShaderAndMetadata(shaderFileName, output);

                if (!output.m_ErrorMessage.empty())
                {
                    AURORA_WARNING("%s", output.m_ErrorMessage.c_str());
                }

                AURORA_INFO("Shader Compiled: %s.", shaderFileName.c_str());

                return m_GraphicsDevice->CreateShader(shaderStage, output.m_ShaderData, output.m_ShaderSize, &shader);
            }
            else
            {
                AURORA_WARNING("%s", output.m_ErrorMessage.c_str());
                return false;
            }
        }

        std::vector<uint8_t> buffer;
        if (FileSystem::PushFileDataToBuffer(shaderFileName, buffer))
        {
            return m_GraphicsDevice->CreateShader(shaderStage, buffer.data(), buffer.size(), &shader);
        }

        return false;
    }

    void Renderer::LoadShaders()
    {
        // Each vertex layout is matched with the shader(s) that use them.

        RendererGlobals::g_InputLayouts[InputLayout_Types::InputLayout_Object_Common].m_Elements =
        {
            { "POSITION_NORMAL_WIND", 0, Vertex_Position::m_Format, InputSlot_PositionNormalWind, RHI_InputLayout::APPEND_ALIGNED_ELEMENT, Input_Per_Vertex_Data },
            { "UVSET",                0, Vertex_TexCoord::m_Format, InputSlot_UV0,                RHI_InputLayout::APPEND_ALIGNED_ELEMENT, Input_Per_Vertex_Data },
            { "UVSET",                1, Vertex_TexCoord::m_Format, InputSlot_UV1,                RHI_InputLayout::APPEND_ALIGNED_ELEMENT, Input_Per_Vertex_Data },
            { "COLOR",                0, Vertex_Color::m_Format,    InputSlot_Color,              RHI_InputLayout::APPEND_ALIGNED_ELEMENT, Input_Per_Vertex_Data },
            { "TANGENT",              0, Vertex_Tangent::m_Format,  InputSlot_Tangent,            RHI_InputLayout::APPEND_ALIGNED_ELEMENT, Input_Per_Vertex_Data },
        };
        LoadShader(Shader_Stage::Vertex_Shader, RendererGlobals::g_Shaders[Shader_Types::VS_Type_Object_Common], "ObjectVS_Common.hlsl");
        LoadShader(Shader_Stage::Pixel_Shader, RendererGlobals::g_Shaders[Shader_Types::PS_Type_Object], "ObjectPS.hlsl");

        RendererGlobals::g_InputLayouts[InputLayout_Types::InputLayout_VertexColor].m_Elements =
        {
            { "POSITION", 0, Format::FORMAT_R32G32B32A32_FLOAT, 0, RHI_InputLayout::APPEND_ALIGNED_ELEMENT, Input_Per_Vertex_Data },
            { "TEXCOORD", 0, Format::FORMAT_R32G32B32A32_FLOAT, 0, RHI_InputLayout::APPEND_ALIGNED_ELEMENT, Input_Per_Vertex_Data },
        };
        LoadShader(Shader_Stage::Vertex_Shader, RendererGlobals::g_Shaders[Shader_Types::VS_Type_VertexColor], "VertexColorVS.hlsl");
        LoadShader(Shader_Stage::Pixel_Shader, RendererGlobals::g_Shaders[Shader_Types::PS_Type_PixelColor], "VertexColorPS.hlsl");


        LoadShader(Shader_Stage::Vertex_Shader, RendererGlobals::g_Shaders[Shader_Types::VS_Type_Sky], "SkyVS.hlsl");
        LoadShader(Shader_Stage::Pixel_Shader, RendererGlobals::g_Shaders[Shader_Types::PS_Type_Sky_Dynamic], "SkyPS_Dynamic.hlsl");
        LoadShader(Shader_Stage::Pixel_Shader, RendererGlobals::g_Shaders[Shader_Types::PS_Type_Sky_Static], "SkyPS_Static.hlsl");
    }

    void Renderer::LoadStates()
    {
        RHI_RasterizerState rasterizerState;
        rasterizerState.m_FillMode = Fill_Mode::Fill_Solid;
        rasterizerState.m_CullMode = Cull_Mode::Cull_Back;
        rasterizerState.m_IsFrontCounterClockwise = true;
        rasterizerState.m_DepthBias = 0;
        rasterizerState.m_DepthBiasClamp = 0;
        rasterizerState.m_DepthBiasSlopeScaled = 0;
        rasterizerState.m_IsDepthClippingEnabled = true;
        rasterizerState.m_IsMultisamplingEnabled = false;
        rasterizerState.m_IsAntialiasedLiningEnabled = false;
        rasterizerState.m_IsConservativeRasterizationEnabled = false;

        RendererGlobals::g_RasterizerStates[RS_Types::RS_Front] = rasterizerState;

        rasterizerState.m_FillMode = Fill_Mode::Fill_Solid;
        rasterizerState.m_CullMode = Cull_Mode::Cull_Front;
        rasterizerState.m_IsFrontCounterClockwise = true;
        rasterizerState.m_DepthBias = 0;
        rasterizerState.m_DepthBiasClamp = 0;
        rasterizerState.m_DepthBiasSlopeScaled = 0;
        rasterizerState.m_IsDepthClippingEnabled = true;
        rasterizerState.m_IsMultisamplingEnabled = false;
        rasterizerState.m_IsAntialiasedLiningEnabled = false;
        rasterizerState.m_IsConservativeRasterizationEnabled = false;

        RendererGlobals::g_RasterizerStates[RS_Types::RS_Back] = rasterizerState;

        rasterizerState.m_FillMode = Fill_Mode::Fill_Solid;
        rasterizerState.m_CullMode = Cull_Mode::Cull_None;
        rasterizerState.m_IsFrontCounterClockwise = true;
        rasterizerState.m_DepthBias = 0;
        rasterizerState.m_DepthBiasClamp = 0;
        rasterizerState.m_DepthBiasSlopeScaled = 0;
        rasterizerState.m_IsDepthClippingEnabled = true;
        rasterizerState.m_IsMultisamplingEnabled = false;
        rasterizerState.m_IsAntialiasedLiningEnabled = false;
        rasterizerState.m_IsConservativeRasterizationEnabled = false;

        RendererGlobals::g_RasterizerStates[RS_Types::RS_DoubleSided] = rasterizerState;

        rasterizerState.m_FillMode = Fill_Mode::Fill_Solid;
        rasterizerState.m_CullMode = Cull_Mode::Cull_Back;
        rasterizerState.m_IsFrontCounterClockwise = true;
        rasterizerState.m_DepthBias = -1;
        rasterizerState.m_DepthBiasClamp = 0;
        rasterizerState.m_DepthBiasSlopeScaled = -4.0f;
        rasterizerState.m_IsDepthClippingEnabled = true;
        rasterizerState.m_IsMultisamplingEnabled = false;
        rasterizerState.m_IsAntialiasedLiningEnabled = false;
        rasterizerState.m_IsConservativeRasterizationEnabled = false;
        
        RendererGlobals::g_RasterizerStates[RS_Types::RS_Shadow] = rasterizerState;
        rasterizerState.m_CullMode = Cull_Mode::Cull_None;
        RendererGlobals::g_RasterizerStates[RS_Types::RS_Shadow_DoubleSided] = rasterizerState;

        rasterizerState.m_FillMode = Fill_Mode::Fill_Wireframe;
        rasterizerState.m_CullMode = Cull_Mode::Cull_Back;
        rasterizerState.m_IsFrontCounterClockwise = true;
        rasterizerState.m_DepthBias = 0;
        rasterizerState.m_DepthBiasClamp = 0;
        rasterizerState.m_DepthBiasSlopeScaled = 0;
        rasterizerState.m_IsDepthClippingEnabled = true;
        rasterizerState.m_IsMultisamplingEnabled = false;
        rasterizerState.m_IsAntialiasedLiningEnabled = false;
        rasterizerState.m_IsConservativeRasterizationEnabled = false;

        RendererGlobals::g_RasterizerStates[RS_Types::RS_Wire] = rasterizerState;
        rasterizerState.m_IsAntialiasedLiningEnabled = true;
        RendererGlobals::g_RasterizerStates[RS_Types::RS_Wire_Smooth] = rasterizerState;

        rasterizerState.m_FillMode = Fill_Mode::Fill_Wireframe;
        rasterizerState.m_CullMode = Cull_Mode::Cull_None;
        rasterizerState.m_IsFrontCounterClockwise = true;
        rasterizerState.m_DepthBias = 0;
        rasterizerState.m_DepthBiasClamp = 0;
        rasterizerState.m_DepthBiasSlopeScaled = 0;
        rasterizerState.m_IsDepthClippingEnabled = true;
        rasterizerState.m_IsMultisamplingEnabled = false;
        rasterizerState.m_IsAntialiasedLiningEnabled = false;
        rasterizerState.m_IsConservativeRasterizationEnabled = false;

        RendererGlobals::g_RasterizerStates[RS_Types::RS_Wire_DoubleSided] = rasterizerState;
        rasterizerState.m_IsAntialiasedLiningEnabled = true;
        RendererGlobals::g_RasterizerStates[RS_Types::RS_Wire_DoubleSided_Smooth] = rasterizerState;

        rasterizerState.m_FillMode = Fill_Mode::Fill_Solid;
        rasterizerState.m_CullMode = Cull_Mode::Cull_Front;
        rasterizerState.m_IsFrontCounterClockwise = true;
        rasterizerState.m_DepthBias = 0;
        rasterizerState.m_DepthBiasClamp = 0;
        rasterizerState.m_DepthBiasSlopeScaled = 0;
        rasterizerState.m_IsDepthClippingEnabled = false;
        rasterizerState.m_IsMultisamplingEnabled = false;
        rasterizerState.m_IsAntialiasedLiningEnabled = false;
        rasterizerState.m_IsConservativeRasterizationEnabled = false;

        RendererGlobals::g_RasterizerStates[RS_Types::RS_Sky] = rasterizerState;

        /// Occludee, Sky, Voxelize.

        RHI_DepthStencilState depthStencilState;
        depthStencilState.m_IsDepthEnabled = true;
        depthStencilState.m_DepthWriteMask = Depth_Write_Mask::Depth_Write_Mask_All;
        depthStencilState.m_DepthComparisonFunction = ComparisonFunction::Comparison_Less; /// Reverse Z?
        depthStencilState.m_IsStencilEnabled = true;
        depthStencilState.m_StencilReadMask = 0;
        depthStencilState.m_StencilWriteMask = 0xFF;
        depthStencilState.m_FrontFaceOperation.m_StencilComparisonFunction = ComparisonFunction::Comparison_Always;
        depthStencilState.m_FrontFaceOperation.m_StencilPassOperation = Stencil_Operation::Stencil_Operation_Replace;
        depthStencilState.m_FrontFaceOperation.m_StencilFailOperation = Stencil_Operation::Stencil_Operation_Keep;
        depthStencilState.m_FrontFaceOperation.m_StencilDepthFailOperation = Stencil_Operation::Stencil_Operation_Keep;
        depthStencilState.m_BackFaceOperation.m_StencilComparisonFunction = ComparisonFunction::Comparison_Always;
        depthStencilState.m_BackFaceOperation.m_StencilPassOperation = Stencil_Operation::Stencil_Operation_Replace;
        depthStencilState.m_BackFaceOperation.m_StencilFailOperation = Stencil_Operation::Stencil_Operation_Keep;
        depthStencilState.m_BackFaceOperation.m_StencilDepthFailOperation = Stencil_Operation::Stencil_Operation_Keep;

        RendererGlobals::g_DepthStencilStates[DS_Types::DS_Default] = depthStencilState;

        depthStencilState.m_IsDepthEnabled = true;
        depthStencilState.m_IsStencilEnabled = false;
        depthStencilState.m_DepthWriteMask = Depth_Write_Mask::Depth_Write_Mask_Zero;
        depthStencilState.m_DepthComparisonFunction = ComparisonFunction::Comparison_Less_Equal;
        
        RendererGlobals::g_DepthStencilStates[DS_Types::DS_DepthRead] = depthStencilState;

        RHI_BlendState blendState;
        blendState.m_RenderTarget[0].m_IsBlendingEnabled = false;
        blendState.m_RenderTarget[0].m_SourceBlendFactor = Blend_Factor::Blend_Source_Alpha;
        blendState.m_RenderTarget[0].m_DestinationBlendFactor = Blend_Factor::Blend_Inverse_Source_Alpha;
        blendState.m_RenderTarget[0].m_BlendOperation = Blend_Operation::Blend_Operation_Maximum;
        blendState.m_RenderTarget[0].m_SourceBlendAlpha = Blend_Factor::Blend_One;
        blendState.m_RenderTarget[0].m_DestinationBlendAlpha = Blend_Factor::Blend_Zero;
        blendState.m_RenderTarget[0].m_BlendOperationAlpha = Blend_Operation::Blend_Operation_Add;
        blendState.m_RenderTarget[0].m_RenderTargetWriteMask = Color_Write_Mask::Color_Write_Enable_All;
        blendState.m_IsAlphaToCoverageEnabled = false;
        blendState.m_IsIndependentBlendingEnabled = false;

        RendererGlobals::g_BlendStates[BS_Types::BS_Opaque] = blendState;

        blendState.m_RenderTarget[0].m_IsBlendingEnabled = true;
        blendState.m_RenderTarget[0].m_SourceBlendFactor = Blend_Factor::Blend_Source_Alpha;
        blendState.m_RenderTarget[0].m_DestinationBlendFactor = Blend_Factor::Blend_Inverse_Source_Alpha;
        blendState.m_RenderTarget[0].m_BlendOperation = Blend_Operation::Blend_Operation_Add;
        blendState.m_RenderTarget[0].m_SourceBlendAlpha = Blend_Factor::Blend_One;
        blendState.m_RenderTarget[0].m_DestinationBlendAlpha = Blend_Factor::Blend_One;
        blendState.m_RenderTarget[0].m_BlendOperationAlpha = Blend_Operation::Blend_Operation_Add;
        blendState.m_RenderTarget[0].m_RenderTargetWriteMask = Color_Write_Mask::Color_Write_Enable_All;
        blendState.m_IsAlphaToCoverageEnabled = false;
        blendState.m_IsIndependentBlendingEnabled = false;

        RendererGlobals::g_BlendStates[BS_Types::BS_Transparent] = blendState;
    }

    void Renderer::LoadBuffers()
    {    
        RHI_GPU_Buffer_Description bufferDescription;

        // The following buffers will be dynamic - short lifetime, fast update, slow read.
        bufferDescription.m_Usage = Usage::Dynamic;
        bufferDescription.m_CPUAccessFlags = CPU_Access::CPU_Access_Write;
        bufferDescription.m_MiscFlags = 0;
        bufferDescription.m_BindFlags = Bind_Flag::Bind_Constant_Buffer;

        bufferDescription.m_ByteWidth = sizeof(ConstantBufferData_Camera);
        m_GraphicsDevice->CreateBuffer(&bufferDescription, nullptr, &RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera]);
        AURORA_INFO("Successfully created Camera Constant Buffer.");

        bufferDescription.m_ByteWidth = sizeof(ConstantBufferData_Misc);
        m_GraphicsDevice->CreateBuffer(&bufferDescription, nullptr, &RendererGlobals::g_ConstantBuffers[CB_Types::CB_Misc]);
        AURORA_INFO("Successfully created Misc Constant Buffer.");

        RHI_Texture_Description textureDescription;
        textureDescription.m_Type = Texture_Type::Texture2D;
        textureDescription.m_Width = 256;
        textureDescription.m_Height = 64;
        textureDescription.m_Format = Format::FORMAT_R16G16B16A16_FLOAT;
        textureDescription.m_BindFlags = Bind_Flag::Bind_Shader_Resource | Bind_Flag::Bind_Unordered_Access;
        m_GraphicsDevice->CreateTexture(&textureDescription, nullptr, &RendererGlobals::g_Textures[Texture_Types::TextureType_2D_SkyAtmosphere_Transmittance_LUT]);

        textureDescription.m_Type = Texture_Type::Texture2D;
        textureDescription.m_Width = 32;
        textureDescription.m_Height = 32;
        textureDescription.m_Format = Format::FORMAT_R16G16B16A16_FLOAT;
        textureDescription.m_BindFlags = Bind_Flag::Bind_Shader_Resource | Bind_Flag::Bind_Unordered_Access;
        m_GraphicsDevice->CreateTexture(&textureDescription, nullptr, &RendererGlobals::g_Textures[Texture_Types::TextureType_2D_SkyAtmosphere_Multiscattered_Luminance_LUT]);

        textureDescription.m_Type = Texture_Type::Texture2D;
        textureDescription.m_Width = 192;
        textureDescription.m_Height = 104;
        textureDescription.m_Format = Format::FORMAT_R16G16B16A16_FLOAT;
        textureDescription.m_BindFlags = Bind_Flag::Bind_Shader_Resource | Bind_Flag::Bind_Unordered_Access;
        m_GraphicsDevice->CreateTexture(&textureDescription, nullptr, &RendererGlobals::g_Textures[Texture_Types::TextureType_2D_SkyAtmosphere_Sky_View_LUT]);

        textureDescription.m_Type = Texture_Type::Texture2D;
        textureDescription.m_Width = 1;
        textureDescription.m_Height = 1;
        textureDescription.m_Format = Format::FORMAT_R16G16B16A16_FLOAT;
        textureDescription.m_BindFlags = Bind_Flag::Bind_Shader_Resource | Bind_Flag::Bind_Unordered_Access;
        m_GraphicsDevice->CreateTexture(&textureDescription, nullptr, &RendererGlobals::g_Textures[Texture_Types::TextureType_2D_SkyAtmosphere_Sky_Luminance_LUT]);
    }

    void Renderer::LoadPipelineStates()
    {
        /*
        RHI_PipelineState_Description pipelineDescription;

        pipelineDescription.m_VertexShader = &RendererGlobals::g_Shaders[Shader_Types::VS_Type_Object_Simple];
        pipelineDescription.m_PixelShader = &RendererGlobals::g_Shaders[Shader_Types::PS_Type_Object_Simple];
        pipelineDescription.m_RasterizerState = &RendererGlobals::g_RasterizerStates[RS_Types::RS_Wire];
        pipelineDescription.m_BlendState = &RendererGlobals::g_BlendStates[BS_Types::BS_Opaque];
        pipelineDescription.m_DepthStencilState = &RendererGlobals::g_DepthStencilStates[DS_Types::DS_Default];

        pipelineDescription.m_InputLayout = &RendererGlobals::g_InputLayouts[InputLayout_Types::InputLayout_Object_Position_TexCoord];

        m_GraphicsDevice->CreatePipelineState(&pipelineDescription, &RendererGlobals::m_PSO_Object_Wire);
        */
        //=========================================================

        RHI_PipelineState_Description objectPipelineDescription;
        objectPipelineDescription.m_VertexShader = &RendererGlobals::g_Shaders[Shader_Types::VS_Type_Object_Common];
        objectPipelineDescription.m_PixelShader = &RendererGlobals::g_Shaders[Shader_Types::PS_Type_Object];
        objectPipelineDescription.m_RasterizerState = &RendererGlobals::g_RasterizerStates[RS_Types::RS_Front];
        objectPipelineDescription.m_BlendState = &RendererGlobals::g_BlendStates[BS_Types::BS_Opaque];
        objectPipelineDescription.m_DepthStencilState = &RendererGlobals::g_DepthStencilStates[DS_Types::DS_Default];
        objectPipelineDescription.m_InputLayout = &RendererGlobals::g_InputLayouts[InputLayout_Types::InputLayout_Object_Common];

        m_GraphicsDevice->CreatePipelineState(&objectPipelineDescription, &RendererGlobals::m_PSO_Object);

        //=========================================================

        RHI_PipelineState_Description skyPipelineDescription;
        skyPipelineDescription.m_RasterizerState = &RendererGlobals::g_RasterizerStates[RS_Types::RS_Sky];
        skyPipelineDescription.m_DepthStencilState = &RendererGlobals::g_DepthStencilStates[DS_Types::DS_DepthRead];
        skyPipelineDescription.m_BlendState = &RendererGlobals::g_BlendStates[BS_Types::BS_Opaque];
        skyPipelineDescription.m_VertexShader = &RendererGlobals::g_Shaders[Shader_Types::VS_Type_Sky];
        skyPipelineDescription.m_PixelShader = &RendererGlobals::g_Shaders[Shader_Types::PS_Type_Sky_Dynamic];

        m_GraphicsDevice->CreatePipelineState(&skyPipelineDescription, &RendererGlobals::m_PSO_Object_Sky[SkyRender_Dynamic]);

        //=========================================================

        RHI_PipelineState_Description skyStaticPipelineDescription;
        skyStaticPipelineDescription.m_RasterizerState = &RendererGlobals::g_RasterizerStates[RS_Types::RS_Sky];
        skyStaticPipelineDescription.m_DepthStencilState = &RendererGlobals::g_DepthStencilStates[DS_Types::DS_DepthRead];
        skyStaticPipelineDescription.m_BlendState = &RendererGlobals::g_BlendStates[BS_Types::BS_Opaque];
        skyStaticPipelineDescription.m_VertexShader = &RendererGlobals::g_Shaders[Shader_Types::VS_Type_Sky];
        skyStaticPipelineDescription.m_PixelShader = &RendererGlobals::g_Shaders[Shader_Types::PS_Type_Sky_Static];

        m_GraphicsDevice->CreatePipelineState(&skyStaticPipelineDescription, &RendererGlobals::m_PSO_Object_Sky[SkyRender_Static]);

        //=========================================================

        RHI_PipelineState_Description debugPipelineDescription;

        debugPipelineDescription.m_VertexShader = &RendererGlobals::g_Shaders[Shader_Types::VS_Type_VertexColor];
        debugPipelineDescription.m_PixelShader = &RendererGlobals::g_Shaders[Shader_Types::PS_Type_PixelColor];
        debugPipelineDescription.m_InputLayout = &RendererGlobals::g_InputLayouts[InputLayout_Types::InputLayout_VertexColor];
        debugPipelineDescription.m_DepthStencilState = &RendererGlobals::g_DepthStencilStates[DS_Types::DS_DepthRead];
        debugPipelineDescription.m_RasterizerState = &RendererGlobals::g_RasterizerStates[RS_Types::RS_Wire_DoubleSided_Smooth];
        debugPipelineDescription.m_BlendState = &RendererGlobals::g_BlendStates[BS_Types::BS_Transparent];
        debugPipelineDescription.m_PrimitiveTopology = Primitive_Topology::LineList;

        m_GraphicsDevice->CreatePipelineState(&debugPipelineDescription, &RendererGlobals::m_PSO_Object_Debug[DebugRenderer_Type::DebugRenderer_Grid]);
    }
}   