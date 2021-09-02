#pragma once
#include <mono/jit/jit.h>
#include "Settings.h"

namespace Aurora::ScriptingUtilities
{
    static Settings* g_SettingsSubsystem = nullptr;

    static std::string ExecuteCommand(const char* command)
    {
        std::array<char, 1024> buffer;
        std::string result;
        const std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);
        if (!pipe)
        {
            AURORA_ERROR(LogLayer::Scripting, "popen() failed");
            return result;
        }

        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }

        return result;
    }

    static bool CompileScript(const std::string& scriptPath, const std::string& dllReference = "")
    {
        // Get Paths
        const std::string scriptDirectory = g_SettingsSubsystem->GetResourceDirectory(ResourceDirectory::Scripts) + "\\";
        const std::string scriptCompiler = scriptDirectory + "mono\\Roslyn\\csc.exe";

        // Compile Script
        std::string command = scriptCompiler + " -target:library -nologo";
        if (!dllReference.empty())
        {
            command += " -reference:" + dllReference;
        }

        command += " -out:" + FileSystem::ReplaceExtension(scriptPath, ".dll") + " " + std::string(scriptPath);
        const std::string result = ExecuteCommand(command.c_str());

        // Log compilation output.
        std::istringstream f(result);
        std::string line;
        bool compilationResult = true;
        while (std::getline(f, line))
        {
            if (FileSystem::IsEmptyOrWhitespace(line))
            {
                continue;
            }

            const auto isError = line.find("error") != std::string::npos;
            if (isError)
            {
                AURORA_ERROR(LogLayer::Scripting, line);
                compilationResult = false;
            }
            else
            {
                AURORA_INFO(LogLayer::Scripting, line);
            }     
        }

        if (compilationResult)
        {
            AURORA_INFO(LogLayer::Scripting, "Successfully compiled C# script: %s", scriptPath.c_str());
        }

        return true;
    }

    static MonoAssembly* CompileAndLoadAssembly(MonoDomain* domain, const std::string& scriptPath, bool isScript = true)
    {
        /*
            // Ensure that the directory of the script contains our callback DLL. Otherwise, Mono will crash.
            if (isScript)
            {
                const std::string callbackDLLPath = g_SettingsSubsystem->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraEngine.dll";
                const std::string callbackScriptLocationWithDLL = FileSystem::GetDirectoryFromFilePath(scriptPath) + "AuroraEngine.dll";
                if (!FileSystem::Exists(callbackScriptLocationWithDLL))
                {
                    FileSystem::CopyFileFromTo(callbackDLLPath, callbackScriptLocationWithDLL);
                }

                // Compile Script
                if (!CompileScript(scriptPath, callbackScriptLocationWithDLL))
                {
                    AURORA_ERROR(LogLayer::Scripting, "Failed to compile script.");
                    return nullptr;
                }
            }
            else // We're loading our callback API.
            {
                // Compile
                if (!CompileScript(scriptPath))
                {
                    AURORA_ERROR(LogLayer::Scripting, "Script compilation failed.");
                    return nullptr;
                }
            }
        */

        // Open assembly.
        const std::string dllPath = FileSystem::ReplaceExtension(scriptPath, ".dll");
        return mono_domain_assembly_open(domain, dllPath.c_str()); // Opens the executable and runs the Main method declared in the executable.
    }

    static MonoMethod* GetMethod(MonoImage* image, const std::string& method)
    {
        // Get method description.
        MonoMethodDesc* monoMethodDescription = mono_method_desc_new(method.c_str(), NULL);
        if (!monoMethodDescription)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to get Method Description for: %s", method.c_str());
            return nullptr;
        }

        // Search for the method in the image.
        MonoMethod* monoMethod = mono_method_desc_search_in_image(monoMethodDescription, image);
        if (!monoMethod)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to get Method for: %s", method.c_str());
            return nullptr;
        }

        return monoMethod;
    }
}