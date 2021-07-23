#include "Aurora.h"
#include "Vector4.h"
#include "Vector3.h"

namespace Aurora::Math
{
    const Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
    const Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);
    const Vector4 Vector4::Infinity(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
    const Vector4 Vector4::InfinityNegative(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

    Vector4::Vector4(const Vector3& otherVector, float w)
    {
        this->x = otherVector.x;
        this->y = otherVector.y;
        this->z = otherVector.z;
        this->w = w;
    }

    Vector4::Vector4(const Vector3& otherVector)
    {
        this->x = otherVector.x;
        this->y = otherVector.y;
        this->z = otherVector.z;
        this->w = 0.0f;
    }

    std::string Vector4::ToString() const
    {
        char temporaryBuffer[200];
        sprintf_s(temporaryBuffer, "X: %f, Y: %f, Z: %f, W: %f", x, y, z, w);
        return std::string(temporaryBuffer);
    }
}