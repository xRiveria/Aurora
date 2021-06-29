#include "Aurora.h"
#include "ShaderCompiler.h"
#include <mutex>
#include <unordered_set>
#include <filesystem>
#include "FileSystem.h"

#include <atlbase.h>
#include <DirectXShaderCompiler/dxcapi.h>
#include <d3dcompiler.h>

namespace Aurora
{
    CComPtr<IDxcUtils> g_DXC_Utilities;
    CComPtr<IDxcCompiler3> g_DXC_Compiler;
    CComPtr<IDxcIncludeHandler> g_DXC_IncludeHandler;
    DxcCreateInstanceProc g_DXC_CreateInstance = nullptr;

    void Compiler_DXCompiler(const CompilerInput& input, CompilerOutput& output)
    {
        if (g_DXC_Compiler == nullptr)
        {
            return;
        }

        std::vector<uint8_t> shaderSourceData;

        // https://github.com/microsoft/DirectXShaderCompiler/wiki/Using-dxc.exe-and-dxcompiler.dll#dxcompiler-dll-interface

        std::vector<LPCWSTR> args = {
            L"-res-may-alias",
            L"-flegacy-macro-expansion",
            //L"-no-legacy-cbuf-layout",
            //L"-pack-optimized",
            //L"-all-resources-bound",
        };

        if (input.m_Flags & Compiler_Flag::Flag_Disable_Optimization)
        {
            args.push_back(L"-Od");
        }

        switch (input.m_Format)
        {
        case Shader_Format::ShaderFormat_HLSL6:
            args.push_back(L"-D"); args.push_back(L"HLSL6");
            break;

        default:
            AURORA_ERROR("Invalid shader format.");
            break;
        }

        args.push_back(L"-T");

        switch (input.m_Stage)
        {
        case Shader_Stage::Vertex_Shader:
        {
            switch (input.m_MinimumShaderModel)
            {
            case Shader_Model::ShaderModel_6_1:
                args.push_back(L"vs_6_1");
                break;
            case Shader_Model::ShaderModel_6_2:
                args.push_back(L"vs_6_2");
                break;
            case Shader_Model::ShaderModel_6_3:
                args.push_back(L"vs_6_3");
                break;
            case Shader_Model::ShaderModel_6_4:
                args.push_back(L"vs_6_4");
                break;
            case Shader_Model::ShaderModel_6_5:
                args.push_back(L"vs_6_5");
                break;
            default:
                args.push_back(L"vs_6_0");
                break;
            }
        }
        break;

        case Shader_Stage::Pixel_Shader:
        {
            switch (input.m_MinimumShaderModel)
            {
            case Shader_Model::ShaderModel_6_1:
                args.push_back(L"ps_6_1");
                break;
            case Shader_Model::ShaderModel_6_2:
                args.push_back(L"ps_6_2");
                break;
            case Shader_Model::ShaderModel_6_3:
                args.push_back(L"ps_6_3");
                break;
            case Shader_Model::ShaderModel_6_4:
                args.push_back(L"ps_6_4");
                break;
            case Shader_Model::ShaderModel_6_5:
                args.push_back(L"ps_6_5");
                break;
            default:
                args.push_back(L"ps_6_0");
                break;
            }
        }
        break;

        default:
            AURORA_ERROR("Invalid shader stage.");
            break;
        }

        std::vector<std::wstring> wStrings;
        wStrings.reserve(input.m_Defines.size() + input.m_IncludeDirectories.size());

        for (const std::string& defineString : input.m_Defines)
        {
            std::wstring& wString = wStrings.emplace_back();
            wString = FileSystem::StringToWString(defineString);
            args.push_back(L"-D"); // Define
            args.push_back(wString.c_str());
        }

        for (const std::string& includeString : input.m_IncludeDirectories)
        {
            std::wstring& wString = wStrings.emplace_back();
            wString = FileSystem::StringToWString(includeString);
            args.push_back(L"-I"); // Include
            args.push_back(wString.c_str());
        }

        // Entry point parameter.
        std::wstring wEntryString = FileSystem::StringToWString(input.m_EntryPoint);
        args.push_back(L"-E");
        args.push_back(wEntryString.c_str());

        // Add source file name as last parameter. This will be displayed in error message.
        std::wstring wSourceString = FileSystem::StringToWString(FileSystem::GetNameFromFilePath(input.m_ShaderSourceFilePath));
        args.push_back(wSourceString.c_str());

        DxcBuffer source;
        source.Ptr = shaderSourceData.data();
        source.Size = shaderSourceData.size();
        source.Encoding = DXC_CP_ACP;

        struct IncludeHandler : public IDxcIncludeHandler
        {
            const CompilerInput* input = nullptr;
            CompilerOutput* output = nullptr;

            HRESULT STDMETHODCALLTYPE LoadSource(
                _In_z_ LPCWSTR pFilename,                                 // Candidate filename.
                _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource  // Resultant source object for included file, nullptr if not found.
            ) override
            {
                HRESULT hr = g_DXC_IncludeHandler->LoadSource(pFilename, ppIncludeSource);
                if (SUCCEEDED(hr))
                {
                    std::string& filename = output->m_Dependencies.emplace_back();
                    filename = FileSystem::WStringToString(pFilename);
                }
                return hr;
            }
            HRESULT STDMETHODCALLTYPE QueryInterface(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
            {
                return g_DXC_IncludeHandler->QueryInterface(riid, ppvObject);
            }

            ULONG STDMETHODCALLTYPE AddRef(void) override
            {
                return 0;
            }
            ULONG STDMETHODCALLTYPE Release(void) override
            {
                return 0;
            }
            } includehandler;

        includehandler.input = &input;
        includehandler.output = &output;

        CComPtr<IDxcResult> pResults;
        HRESULT hr = g_DXC_Compiler->Compile(
            &source,                // Source buffer.
            args.data(),            // Array of pointers to arguments.
            (uint32_t)args.size(),	// Number of arguments.
            &includehandler,		// User-provided interface to handle #include directives (optional).
            IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
        );
        assert(SUCCEEDED(hr));

        CComPtr<IDxcBlobUtf8> pErrors = nullptr;
        hr = pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
        assert(SUCCEEDED(hr));
        if (pErrors != nullptr && pErrors->GetStringLength() != 0)
        {
            output.m_ErrorMessage = pErrors->GetStringPointer();
        }

        HRESULT hrStatus;
        hr = pResults->GetStatus(&hrStatus);
        assert(SUCCEEDED(hr));
        if (FAILED(hrStatus))
        {
            return;
        }

        CComPtr<IDxcBlob> pShader = nullptr;
        hr = pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), nullptr);
        assert(SUCCEEDED(hr));
        if (pShader != nullptr)
        {
            output.m_Dependencies.push_back(input.m_ShaderSourceFilePath);
            output.m_ShaderData = (const uint8_t*)pShader->GetBufferPointer();
            output.m_ShaderSize = pShader->GetBufferSize();

            // keep the blob alive == keep shader pointer valid!
            auto internal_state = std::make_shared<CComPtr<IDxcBlob>>();
            *internal_state = pShader;
            output.m_InternalState = internal_state;
        }

        if (input.m_Format == Shader_Format::ShaderFormat_HLSL6)
        {
            CComPtr<IDxcBlob> pHash = nullptr;
            hr = pResults->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&pHash), nullptr);
            assert(SUCCEEDED(hr));
            if (pHash != nullptr)
            {
                DxcShaderHash* pHashBuf = (DxcShaderHash*)pHash->GetBufferPointer();
                for (int i = 0; i < _countof(pHashBuf->HashDigest); i++)
                {
                    output.m_ShaderHash.push_back(pHashBuf->HashDigest[i]);
                }
            }
        }
    }


    void ShaderCompiler::Initialize()
    {

    }

    void ShaderCompiler::Compile(const CompilerInput& input, const CompilerOutput& output)
    {
    }

    bool ShaderCompiler::SaveShaderAndMetadata(const std::string& shaderFileName, const CompilerOutput& output)
    {
        return false;
    }

    bool ShaderCompiler::IsShaderOutdated(const std::string& shaderFileName)
    {
        return false;
    }

    void ShaderCompiler::RegisterShader(const std::string& shaderFileName)
    {
    }

    bool ShaderCompiler::CheckIfRegisteredShadersAreOutdated()
    {
        return false;
    }
}