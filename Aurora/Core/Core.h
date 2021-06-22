#pragma once

#define AURORA_DEBUG_BREAK() __debugbreak()

#define AURORA_ASSERT(expression)  \
if (!(##expression))               \
{                                  \
    AURORA_ERROR(#expression);     \
    AURORA_DEBUG_BREAK();          \
}