#pragma once

#include "MathUtilities.h"
#include <string>

namespace Aurora::Math
{
    class Vector3
    {
    public:
        Vector3()
        {
            x = 0;
            y = 0;
            z = 0;
        }

        Vector3(const Vector3& otherVector)
        {
            this->x = otherVector.x;
            this->y = otherVector.y;
            this->z = otherVector.z;
        }

        // Vector3(const Vector4& otherVector);

        Vector3(float x, float y, float z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        Vector3(float values[3])
        {
            this->x = values[0];
            this->y = values[1];
            this->z = values[2];
        }

        Vector3(float value)
        {
            this->x = value;
            this->y = value;
            this->z = value;
        }

        // Returns the dot product. If the dot product is 0, the vectors are orthogonal.
        static inline float Dot(const Vector3& vectorA, const Vector3& vectorB)
        {
            return (vectorA.x * vectorB.x + vectorA.y * vectorB.y + vectorA.z * vectorB.z);
        }

        // Returns the dot product.
        float Dot(const Vector3& otherVector) const { return this->x * otherVector.x + this->y * otherVector.y + this->z * otherVector.z; }

        // Returns the cross product. Returns a third vector perpendicular/orthogonal to the original two axis.
        static inline Vector3 Cross(const Vector3& vectorA, const Vector3& vectorB)
        {
            // i, j, k in a matrix.
            return Vector3(
                vectorA.y * vectorB.z - vectorB.y * vectorA.z,
                -(vectorA.x * vectorB.z - vectorB.x * vectorA.z),
                vectorA.x * vectorB.y - vectorB.x * vectorA.y
            );
        }

        // Returns the cross product.
        Vector3 Cross(const Vector3& otherVector) const { return Cross(*this, otherVector); }

        // Returns the length.
        float Length() const { return Helper::SquareRoot(x * x + y * y + z * z); }

        // Returns the length squared.
        float LengthSquared() const { return x * x + y * y + z * z; }

        // Distance from this vector to the other vector.
        inline float Distance(const Vector3& otherVector) { return ((*this) - otherVector).Length(); }

        // Distance from this vector to the other vector squared.
        inline float DistanceSquared(const Vector3& otherVector) { return ((*this) - otherVector).LengthSquared(); }

        // Returns the distance between the two vectors.
        static inline float Distance(const Vector3& vectorA, const Vector3& vectorB) { return (vectorB - vectorA).Length(); }

        // Returns the squared distance between the two vectors.
        static inline float DistanceSquared(const Vector3& vectorA, const Vector3& vectorB) { return (vectorB - vectorA).LengthSquared(); }

        // Normalize to Unit Vector.
        void Normalize()
        {
            const float lengthSquared = LengthSquared();

            // We will only normalize the vector if it's current length is not already 1.0 and more than 0.
            if (!Helper::Equals(lengthSquared, 1.0f) && lengthSquared > 0.0f)
            {
                const float lengthInverted = 1.0f / Helper::SquareRoot(lengthSquared);

                this->x *= lengthInverted;
                this->y *= lengthInverted;
                this->z *= lengthInverted;
            }
        }

        // Returns normalized vector.
        static inline Vector3 Normalize(const Vector3& otherVector) { return otherVector.Normalized(); }

        Vector3 Normalized() const
        {
            const float lengthSquared = LengthSquared();

            if (!Helper::Equals(lengthSquared, 1.0f) && lengthSquared > 0.0f)
            {
                const float lengthInverted = 1.0f / Helper::SquareRoot(lengthSquared);
                return (*this) * lengthInverted;
            }
            else
            {
                return *this;
            }
        }

        // Multiplication
        Vector3 operator*(const Vector3& otherVector) const
        {
            return Vector3(
                this->x * otherVector.x,
                this->y * otherVector.y,
                this->z * otherVector.z
            );
        }

        Vector3 operator*(const float value) const
        {
            return Vector3(
                this->x * value,
                this->y * value,
                this->z * value
            );
        }

        void operator*=(const Vector3& otherVector)
        {
            this->x *= otherVector.x;
            this->y *= otherVector.y;
            this->z *= otherVector.z;
        }

        void operator*=(const float value)
        {
            this->x *= value;
            this->y *= value;
            this->z *= value;
        }

        // Addition
        Vector3 operator+(const Vector3& otherVector) const
        {
            return Vector3(this->x + otherVector.x, this->y + otherVector.y, this->z + otherVector.z);
        }

        Vector3 operator+(const float value) const
        {
            return Vector3(this->x + value, this->y + value, this->z + value);
        }

        void operator+=(const Vector3& otherVector)
        {
            this->x += otherVector.x;
            this->y += otherVector.y;
            this->z += otherVector.z;
        }

        void operator+=(const float value)
        {
            this->x += value;
            this->y += value;
            this->z += value;
        }

        // Subtraction
        Vector3 operator-(const Vector3& otherVector) const
        {
            return Vector3(this->x - otherVector.x, this->y - otherVector.y, this->z - otherVector.z);
        }

        Vector3 operator-(const float value) const
        {
            return Vector3(this->x - value, this->y - value, this->z - value);
        }

        void operator-=(const Vector3& otherVector)
        {
            this->x -= otherVector.x;
            this->y -= otherVector.y;
            this->z -= otherVector.z;
        }

        void operator-=(const float value)
        {
            this->x -= value;
            this->y -= value;
            this->z -= value;
        }

        // Division
        Vector3 operator/(const Vector3& otherVector) const
        {
            return Vector3(this->x / otherVector.x, this->y / otherVector.y, this->z / otherVector.z);
        }

        Vector3 operator/(const float value) const
        {
            return Vector3(this->x / value, this->y / value, this->z / value);
        }

        void operator/=(const Vector3& otherVector)
        {
            x /= otherVector.x;
            y /= otherVector.y;
            z /= otherVector.z;
        }

        void operator/=(const float value)
        {
            x /= value;
            y /= value;
            z /= value;
        }

        // Test for equality without using epsilon.
        bool operator==(const Vector3& otherVector) const
        {
            return this->x == otherVector.x && this->y == otherVector.y && this->z == otherVector.z;
        }

        // Test for inequality without using epsilon.
        bool operator!=(const Vector3& otherVector) const
        {
            return !(*this == otherVector);
        }

        // Return negation.
        Vector3 operator-() const { return Vector3(-this->x, -this->y, -this->z); }
        
        float* Data() { return &x; }
        std::string ToString() const;

    public:
        float x = 0;
        float y = 0;
        float z = 0;

        static const Vector3 Zero;
        static const Vector3 Left;
        static const Vector3 Right;
        static const Vector3 Up;
        static const Vector3 Down;
        static const Vector3 Forward;
        static const Vector3 Backward;
        static const Vector3 One;
        static const Vector3 Infinity;
        static const Vector3 InfinityNegative;
    };

    // Reverse order operators.
    // inline Vector3 operator*(float value, const Vector3& otherVector) { return otherVector * value; }
}