#pragma once
#include "../Graphics/RHI_Utilities.h"

namespace Aurora::ShaderCompiler
{
    enum Compiler_Flag
    {
        Flag_None = 0,
        Flag_Disable_Optimization = 1 << 0
    };

    struct CompilerInput
    {
        uint64_t m_Flags = Compiler_Flag::Flag_None;
        Shader_Format m_Format = Shader_Format::ShaderFormat_None;
        RHI_Shader_Stage m_Stage = RHI_Shader_Stage::ShaderStage_Count;
        Shader_Model m_MinimumShaderModel = Shader_Model::ShaderModel_5_0; // If the shader relies on a higher shader model feature, it must be declared here. However, the compiler can also choose a higher one internally if needed.
        std::string m_ShaderSourceFilePath;
        std::string m_EntryPoint = "main";
        std::vector<std::string> m_IncludeDirectories;
        std::vector<std::string> m_Defines;
    };

    struct CompilerOutput
    {
        std::shared_ptr<void> m_InternalState;
        inline bool IsValid() const { return m_InternalState.get() != nullptr; }
        const uint8_t* m_ShaderData = nullptr;
        size_t m_ShaderSize = 0;
        std::vector<uint8_t> m_ShaderHash;
        std::string m_ErrorMessage;
        std::vector<std::string> m_Dependencies;
    };

    class ShaderCompiler
    {
    public:
        void Initialize();

        void Compile(const CompilerInput& input, CompilerOutput& output);

        bool SaveShaderAndMetadata(const std::string& shaderFileName, const CompilerOutput& output);
        bool IsShaderOutdated(const std::string& shaderFileName);

        void RegisterShader(const std::string& shaderFileName);
        bool CheckIfRegisteredShadersAreOutdated();
    };
}