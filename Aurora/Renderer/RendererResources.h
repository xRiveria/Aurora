#pragma once
#include "Renderer.h"
#include "FileSystem.h"
#include "../Graphics/DX11/DX11_Utilities.h"
#include "../Scene/World.h"

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

        // Pipeline States
        RHI_PipelineState m_PSO_Object_Wire;

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
        LoadShader(Shader_Stage::Vertex_Shader, m_VertexShader, "TriangleVS.hlsl");
        LoadShader(Shader_Stage::Pixel_Shader, m_PixelShader, "TrianglePS.hlsl");

        // These input layouts are created on pipeline state creation.
        RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle].m_Elements =
        {
            { "POS",      0, Format::FORMAT_R32G32B32_FLOAT, 0, 0,  Input_Classification::Input_Per_Vertex_Data  },
            { "TEXCOORD", 0, Format::FORMAT_R32G32_FLOAT,    0, 12, Input_Classification::Input_Per_Vertex_Data },
            { "NORMAL",   0, Format::FORMAT_R32G32B32_FLOAT, 0, 20, Input_Classification::Input_Per_Vertex_Data }
        };
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
    }

    void Renderer::LoadPipelineStates()
    {
        RHI_PipelineState_Description pipelineDescription;

        pipelineDescription.m_VertexShader = &m_VertexShader;
        pipelineDescription.m_PixelShader = &m_PixelShader;
        pipelineDescription.m_RasterizerState = &RendererGlobals::g_RasterizerStates[RS_Types::RS_Front];
        pipelineDescription.m_BlendState = &RendererGlobals::g_BlendStates[BS_Types::BS_Opaque];
        pipelineDescription.m_DepthStencilState = &RendererGlobals::g_DepthStencilStates[DS_Types::DS_Default];

        pipelineDescription.m_InputLayout = &RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle];

        m_GraphicsDevice->CreatePipelineState(&pipelineDescription, &RendererGlobals::m_PSO_Object_Wire);
    }
}