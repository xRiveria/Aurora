#pragma once
#include "Renderer.h"
#include "FileSystem.h"
#include "../Graphics/DX11/DX11_Utilities.h"

namespace Aurora
{
    namespace RendererGlobals
    {
        RHI_GPU_Buffer   g_ConstantBuffers[CB_Count];
        RHI_InputLayout  g_InputLayouts[InputLayout_Count];

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

        
        std::vector<D3D11_INPUT_ELEMENT_DESC> description(RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle].m_Elements.size());

        for (size_t i = 0; i < description.size(); ++i)
        {
            description[i].SemanticName = RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle].m_Elements[i].m_SemanticName.c_str();
            description[i].SemanticIndex = RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle].m_Elements[i].m_SemanticIndex;
            description[i].Format = DX11_Utility::DX11_ConvertFormat(RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle].m_Elements[i].m_Format);
            description[i].InputSlot = RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle].m_Elements[i].m_InputSlot;
            description[i].AlignedByteOffset = RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle].m_Elements[i].m_AlignedByteOffset;
            description[i].InputSlotClass = DX11_Utility::DX11_ConvertInputClassification(RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle].m_Elements[i].m_InputSlotClass);
            description[i].InstanceDataStepRate = 0;

            if (description[i].InputSlotClass == D3D11_INPUT_PER_INSTANCE_DATA)
            {
                description[i].InstanceDataStepRate = -1;
            }
        }

        // We usually create this with the pipeline state. For now, we will create it here.
        auto vs_internal = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_VertexShader.m_InternalState.get());
        m_GraphicsDevice->m_Device->CreateInputLayout(description.data(), (UINT)description.size(), vs_internal->m_ShaderCode.data(), vs_internal->m_ShaderCode.size(), &m_InputLayout);      
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

    inline void Renderer::LoadPipelineStates()
    {

    }
}