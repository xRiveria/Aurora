#pragma once
#include <string>
#include "MathUtilities.h"

namespace Aurora::Math
{
    class Vector3;

    class Vector4
    {
    public:
        Vector4()
        {
            this->x = 0.0f;
            this->y = 0.0f;
            this->z = 0.0f;
            this->w = 0.0f;
        }

        Vector4(float x, float y, float z, float w)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }

        Vector4(float value)
        {
            this->x = value;
            this->y = value;
            this->z = value;
            this->w = value;
        }

        Vector4(const Vector3& otherVector, float w);
        Vector4(const Vector3& otherVector);

        ~Vector4() = default;

        bool operator==(const Vector4& otherVector) const
        {
            return this->x == otherVector.x && this->y == otherVector.y && this->z == otherVector.z && this->w == otherVector.w;
        }

        bool operator!=(const Vector4& otherVector) const
        {
            return !(*this == otherVector);
        }

        Vector4 operator*(const float value) const
        {
            return Vector4(
                this->x * value,
                this->y * value,
                this->z * value,
                this->w * value
            );
        }

        void operator*=(const float value)
        {
            this->x *= value;
            this->y *= value;
            this->z *= value;
            this->w *= value;
        }

        Vector4 operator/(const float value) const
        {
            return Vector4(this->x / value, this->y / value, this->z / value, this->w / value);
        }

        // Returns the length.
        float Length() const { return Utilities::SquareRoot(this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w); }
        
        // Returns the squared length.
        float LengthSquared() const { return this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w; }

        // Normalize
        void Normalize()
        {
            const float lengthSquared = LengthSquared();

            if (!Utilities::Equals(lengthSquared, 1.0f) && lengthSquared > 0.0f)
            {
                const float lengthInverted = 1.0f / Utilities::SquareRoot(lengthSquared);

                this->x *= lengthInverted;
                this->y *= lengthInverted;
                this->z *= lengthInverted;
                this->w *= lengthInverted;
            }
        }

        Vector4 Normalized() const
        {
            const float lengthSquared = LengthSquared();

            if (!Utilities::Equals(lengthSquared, 1.0f) && lengthSquared > 0.0f)
            {
                const float lengthInverted = 1.0f / Utilities::SquareRoot(lengthSquared);
                return (*this) * lengthInverted;
            }
            else
            {
                return *this;
            }
        }

        std::string ToString() const;
        float* Data() { return &x; }

    public:
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;

        static const Vector4 One;
        static const Vector4 Zero;
        static const Vector4 Infinity;
        static const Vector4 InfinityNegative;
    };
}