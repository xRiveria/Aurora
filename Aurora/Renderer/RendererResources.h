#pragma once

namespace Aurora
{
    namespace RendererGlobals
    {
        RHI_GPU_Buffer g_ConstantBuffers[CB_Count];
    }

    void Renderer::LoadShader(Shader_Stage shaderStage, RHI_Shader& shader, const std::string& filePath, Shader_Model minimumShaderModel)
    {
        
    }

    void Renderer::LoadShaders()
    {

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
}