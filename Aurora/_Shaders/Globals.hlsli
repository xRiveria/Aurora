#ifndef AURORA_SHADER_GLOBALS_HF
#define AURORA_SHADER_GLOBALS_HF

#include "ShaderUtilities.h"
#include "ShaderInternals.h"

TEXTURECUBE(texture_global_environmental_map, float4, TEXSLOT_GLOBAL_ENVIRONMENTAL_MAP);

// Creates a full screen triangle from 3 vertices.
inline void FullScreenTriangle(in uint vertexID, out float4 position)
{
    position.x = (float)(vertexID / 2) * 2.5 - 1.0;
    position.y = (float)(vertexID % 2) * 2.5 - 1.0;
    position.z = 0;
    position.w = 1;
}

#endif //AURORA_SHADER_GLOBALS_HF
