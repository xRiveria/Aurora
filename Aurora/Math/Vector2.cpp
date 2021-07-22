#include "Aurora.h"
#include "Vector2.h"

namespace Aurora::Math
{
    const Vector2 Vector2::Zero(0.0f, 0.0f);
    const Vector2 Vector2::One(1.0f, 1.0f);

    std::string Vector2::ToString() const
    {
        char temporaryBuffer[200];
        sprintf_s(temporaryBuffer, "X: %f, Y: %f", x, y);
        return std::string(temporaryBuffer);
    }
}