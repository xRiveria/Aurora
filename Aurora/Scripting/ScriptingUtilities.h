#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/debug-helpers.h>
#include "Settings.h"
#include "FileSystem.h"
#include <array>

namespace Aurora::ScriptingUtilities
{
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

    static bool CompileScript(const std::string& scriptPath, const std::string& dllReference = "", Settings* settingsSubsystem = nullptr)
    {
        // Get Paths
        const std::string scriptDirectory = settingsSubsystem->GetResourceDirectory(ResourceDirectory::Scripts) + "\\";
        const std::string scriptCompiler = scriptDirectory + "mono\\Roslyn\\csc.exe";

        // Compile ScriptInstance
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
                AURORA_ERROR(LogLayer::Scripting, line);
            }
        }

        if (compilationResult)
        {
            AURORA_INFO(LogLayer::Scripting, "Successfully compiled C# script: %s", scriptPath.c_str());
        }

        return true;
    }
}

    /*
    static std::vector<char> ReadFile(const std::string& filePath)
    {
        std::ifstream myfile(filePath.c_str(), std::ios::binary | std::ios::ate);
        std::ifstream::pos_type pos = myfile.tellg();

        std::vector<char> result(pos);
        myfile.seekg(0, std::ios::beg);
        myfile.read(&result[0], pos);

        return result;
    }

    static std::pair<MonoAssembly*, MonoImage*> LoadAssembly(const std::string& filePath)
    {
        AURORA_INFO(LogLayer::Scripting, "Loading Assembly for: %s", filePath.c_str());
        const std::string dllPath = FileSystem::ReplaceExtension(filePath, ".dll");

        // Read the DLL file into memory.
        std::vector<char> data = ReadFile(dllPath);

        // Open as image.
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_with_name((char*)&data[0], data.size(), true, &status, false, dllPath.c_str());
        if (status != MONO_IMAGE_OK || image == 0)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load mono image from DLL file...");
            return std::pair(nullptr, nullptr);
        }

        // Create image from assembly.
        //MonoAssembly* assembly = mono_image_get_assembly(image);
        //if (assembly == NULL)
        //{
            MonoAssembly* assembly = mono_assembly_load_from_full(image, dllPath.c_str(), &status, false);
        //}
        if (status != MONO_IMAGE_OK || assembly == 0)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load mono image from DLL file...");
            return std::pair(nullptr, nullptr);
        }

        return std::pair(assembly, image);
    }

    inline std::pair<MonoAssembly*, MonoImage*> CompileAndLoadAssembly(MonoDomain* domain, const std::string& scriptPath, bool isScript = true)
    {
        const std::string dllPath = FileSystem::ReplaceExtension(scriptPath, ".dll");

        // Ensure that the directory of the script contains our callback DLL. Otherwise, Mono will crash.
        if (isScript)
        {
            const std::string callbackDLLPath = g_SettingsSubsystem->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraEngine.dll";
            const std::string callbackScriptLocationWithDLL = FileSystem::GetDirectoryFromFilePath(scriptPath) + "AuroraEngine.dll"; // Does our script directory have our DLL file?
            if (!FileSystem::Exists(callbackScriptLocationWithDLL)) // If it doesn't...
            {
                FileSystem::CopyFileFromTo(callbackDLLPath, callbackScriptLocationWithDLL); // Copy our callback file to the script's location.
            }

            // Compile ScriptInstance
            if (!CompileScript(scriptPath, callbackScriptLocationWithDLL))
            {
                AURORA_ERROR(LogLayer::Scripting, "Failed to compile script.");
                std::pair(nullptr, nullptr);
            }
        }
        else // We're loading our callback API.
        {
            // Compile
            if (!CompileScript(scriptPath))
            {
                AURORA_ERROR(LogLayer::Scripting, "ScriptInstance compilation failed.");
                std::pair(nullptr, nullptr);
            }
        }

        return LoadAssembly(scriptPath);
        /*
        // ======================
        // Load assembly from memory as mono_domain_assembly_open keeps a lock on the file.
        // Read our assembly.
        char* assembly_data = nullptr;
        uint32_t assembly_data_size = 0;
        if (FileRead(dllPath.c_str(), assembly_data, assembly_data_size))
        {
            AURORA_INFO(LogLayer::Scripting, "Loading %s into Domain...", dllPath.c_str());
            MonoImageOpenStatus status;

            // Open the assembly from the data we read, so we never lock files.
            MonoImage* image = mono_image_open_from_data_with_name(assembly_data, assembly_data_size, true, &status, false, dllPath.c_str());
            if (status != MONO_IMAGE_OK || image == nullptr)
            {
                AURORA_ERROR(LogLayer::Scripting, "Failed loading assembly %s\n", dllPath.c_str());
                return nullptr;
            }

            MonoAssembly* assembly = mono_assembly_load_from_full(image, dllPath.c_str(), &status, false);

            // Decrement reference count previously incremented with mono_imagE_open_from_data_with_name.
            mono_image_close(image);

            return assembly;
        }

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
*/