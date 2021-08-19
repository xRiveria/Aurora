#include "ShaderUtilities.h"
#include "ShaderInternals.h"
// Use these to define the expected input layout for the shader.

#ifdef SHADER_LAYOUT_POSITION_COLOR
#define SHADER_INPUT_COLOR
#endif

#ifdef SHADER_USE_POSITION_COLOR
#define SHADER_USE_COLOR
#endif

struct VertexInput
{
    float3 position : POSITION;

#ifdef SHADER_INPUT_COLOR
    float4 color : COLOR;
#endif
};

struct PixelInput
{
    float4 position : SV_POSITION;

#ifdef SHADER_USE_COLOR
    float4 color : COLOR;
#endif
};