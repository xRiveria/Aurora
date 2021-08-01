#pragma once
#include <string>

#define AURORA_DEBUG_BREAK() __debugbreak()

#define AURORA_ASSERT(expression)  \
if (!(##expression))               \
{                                  \
    AURORA_ERROR(#expression);     \
    AURORA_DEBUG_BREAK();          \
}

#define AURORA_GRAPHICS_DX11
#define NOMINMAX

// ===== Engine Information ======

#define ENGINE_NAME "Aurora Engine"
#define ENGINE_BUILD "1.0"

// Currently selected Graphics API.
#define ENGINE_GRAPHICS_API GetGraphicsAPI()

inline static std::string GetGraphicsAPI()
{
    return "DirectX 11"; // We only support DX11 for now.
}

// Checks if build is in Debug or Release mode.
#if defined _DEBUG 
    #define ENGINE_BUILD_TYPE "Debug"
#else
    #define ENGINE_BUILD_TYPE "Release"
#endif

// Checks the engine's compilation target. We currently only support Windows.

#if defined(_WIN64)
    #define ENGINE_COMPILATION_TARGET "Windows (x64)"
#else
    #define ENGINE_COMPILATION_TARGET "Windows (x32)"
#endif

#define ENGINE_ARCHITECTURE std::string(ENGINE_NAME) + " - " + std::string(ENGINE_COMPILATION_TARGET) + " " + std::string(ENGINE_BUILD_TYPE) + " - " + std::string(GetGraphicsAPI())

// ==========

// Future Graphics APIs
// #define AURORA_GRAPHICS_DX11
// #define AURORA_GRAPHICS_DX11

// Serializations
namespace YAML
{
    class Emitter;
    class Node;
}

typedef YAML::Emitter SerializationStream;
typedef YAML::Node    SerializationNode;

