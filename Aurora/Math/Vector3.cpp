#include "Aurora.h"
#include "Vector3.h"

namespace Aurora::Math
{
    const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
    const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
    const Vector3 Vector3::Left(-1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::Right(1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::Forward(0.0f, 0.0f, 1.0f); // Left handed coordinates.
    const Vector3 Vector3::Backward(0.0f, 0.0f, -1.0f);
    const Vector3 Vector3::Up(0.0f, 1.0f, 0.0f);
    const Vector3 Vector3::Down(0.0f, -1.0f, 0.0f);
    const Vector3 Vector3::Infinity(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
    const Vector3 Vector3::InfinityNegative(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

    std::string Vector3::ToString() const
    {
        char buffer[200];
        sprintf_s(buffer, "X: %f, Y: %f, Z: %f", x, y, z);
        return std::string(buffer);
    }
}