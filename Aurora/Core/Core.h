#pragma once

#define AURORA_DEBUG_BREAK() __debugbreak()

#define AURORA_ASSERT(expression)  \
if (!(##expression))               \
{                                  \
    AURORA_ERROR(#expression);     \
    AURORA_DEBUG_BREAK();          \
}

#define AURORA_GRAPHICS_DX11
#define NOMINMAX

// Serializations
namespace YAML
{
    class Emitter;
    class Node;
}

typedef YAML::Emitter SerializationStream;
typedef YAML::Node    SerializationNode;

// Future Graphics APIs
// #define AURORA_GRAPHICS_DX11
// #define AURORA_GRAPHICS_DX11