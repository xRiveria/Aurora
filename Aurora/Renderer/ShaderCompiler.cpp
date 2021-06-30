#include "Aurora.h"
#include "ShaderCompiler.h"
#include <mutex>
#include <unordered_set>
#include <filesystem>
#include "FileSystem.h"

#include <atlbase.h>
#include <DirectXShaderCompiler/dxcapi.h>
#include <d3dcompiler.h>

namespace Aurora::ShaderCompiler
{
    using PFN_D3DCOMPILE = decltype(&D3DCompile);
    PFN_D3DCOMPILE D3DCompile = nullptr;

    void Compile_D3DCompiler(const CompilerInput& input, CompilerOutput& output)
    {
        if (D3DCompile == nullptr)
        {
            return;
        }

        if (input.m_MinimumShaderModel > Shader_Model::ShaderModel_5_0)
        {
            output.m_ErrorMessage = "HLSL5 cannot support the specified minimum shader model.";
            return;
        }

        std::vector<uint8_t> shaderSourceData;
        if (!FileSystem::PushFileDataToBuffer(input.m_ShaderSourceFilePath, shaderSourceData))
        {
            return;
        }

        D3D_SHADER_MACRO defines[] = {
            "HLSL5", "1",
            "DISABLE_WAVE_INTRINSICS", "1",
            NULL, NULL,
        };

        const char* target = nullptr;
        switch (input.m_Stage)
        {
            case Shader_Stage::Vertex_Shader:
                target = "vs_5_0";
                break;

            case Shader_Stage::Pixel_Shader:
                target = "ps_5_0";
                break;

            default:
                AURORA_ERROR("Shader not supported.");
                break;
        }

        struct IncludeHandler : public ID3DInclude
        {
            const CompilerInput* input = nullptr;
            CompilerOutput* output = nullptr;
            std::vector<std::vector<uint8_t>> fileDatas;

            HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override
            {
                for (const std::string& include : input->m_IncludeDirectories)
                {
                    std::string fileName = include + pFileName;

                    if (!FileSystem::Exists(fileName))
                    {
                        continue;
                    }

                    std::vector<uint8_t>& fileData = fileDatas.emplace_back();
                    if (FileSystem::PushFileDataToBuffer(fileName, fileData))
                    {
                        output->m_Dependencies.push_back(fileName);
                        *ppData = fileData.data();
                        *pBytes = (UINT)fileData.size();
                        return S_OK;
                    }
                }
                return E_FAIL;
            }

            HRESULT Close(LPCVOID pData) override
            {
                return S_OK;
            }

        } includeHandler;

        includeHandler.input = &input;
        includeHandler.output = &output;

        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/d3dcompile-constants
        UINT Flags1 = 0;
        if (input.m_Flags & Compiler_Flag::Flag_Disable_Optimization)
        {
            Flags1 |= D3DCOMPILE_SKIP_OPTIMIZATION;
        }

        CComPtr<ID3DBlob> code;
        CComPtr<ID3DBlob> errors;

        HRESULT result = D3DCompile(shaderSourceData.data(), shaderSourceData.size(), input.m_ShaderSourceFilePath.c_str(), defines, &includeHandler,
            input.m_EntryPoint.c_str(), target, Flags1, 0, &code, &errors);

        if (errors)
        {
            output.m_ErrorMessage = (const char*)errors->GetBufferPointer();
        }

        if (SUCCEEDED(result))
        {
            output.m_Dependencies.push_back(input.m_ShaderSourceFilePath);
            output.m_ShaderData = (const uint8_t*)code->GetBufferPointer();
            output.m_ShaderSize = code->GetBufferSize();

            // Keep the blob alive == keep shader pointer valid.
            auto internalState = std::make_shared<CComPtr<ID3D10Blob>>();
            *internalState = code;
            output.m_InternalState = internalState;
        }
    }

    void ShaderCompiler::Initialize()
    {
        // If D3DCompiler is requested.
        HMODULE d3dCompiler = LoadLibrary(_T("d3dcompiler_47.dll"));
        if (d3dCompiler != nullptr)
        {
            D3DCompile = (PFN_D3DCOMPILE)GetProcAddress(d3dCompiler, "D3DCompile");
            if (D3DCompile != nullptr)
            {
                AURORA_INFO("Successfully loaded d3dCompiler_47.dll");
            }
            else
            {
                AURORA_ERROR("Failed to load d3dCompiler_47.dll");
            }
        }
    }

    void ShaderCompiler::Compile(const CompilerInput& input, CompilerOutput& output)
    {
        output = CompilerOutput(); // Set to a fresh instance.

        // We will switch compilers according to the currently utilized graphics device.
        switch (input.m_Format)
        {
            case Shader_Format::ShaderFormat_HLSL5:
                Compile_D3DCompiler(input, output);
                break;

            default:
                AURORA_ERROR("Shader compiling for requested shader format is invalid. Aborting...");
                break;
        }
    }

    bool ShaderCompiler::SaveShaderAndMetadata(const std::string& shaderFileName, const CompilerOutput& output)
    {
        /// Always pretend to be saved for now.
        return true;
    }

    bool ShaderCompiler::IsShaderOutdated(const std::string& shaderFileName)
    {
        /// Always outdated for now.
        return true;    
    }

    void ShaderCompiler::RegisterShader(const std::string& shaderFileName)
    {
        /// Empty for now.
    }

    bool ShaderCompiler::CheckIfRegisteredShadersAreOutdated()
    {
        /// Always outdated for now.
        return true;
    }
}