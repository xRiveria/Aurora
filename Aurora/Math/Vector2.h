#pragma once
#include <memory>
#include "MathUtilities.h"

namespace Aurora::Math
{
    class Vector2
    {
    public:
        Vector2()
        {
            x = 0;
            y = 0;
        }

        Vector2(const Vector2& otherVector)
        {
            this->x = otherVector.x;
            this->y = otherVector.y;
        }

        Vector2(float x, float y)
        {
            this->x = x;
            this->y = y;
        }

        Vector2(int x, int y)
        {
            this->x = static_cast<float>(x);
            this->y = static_cast<float>(y);
        }

        Vector2(uint32_t x, uint32_t y)
        {
            this->x = static_cast<float>(x);
            this->y = static_cast<float>(y);
        }

        Vector2(float x)
        {
            this->x = x;
            this->y = y;
        }

        ~Vector2() = default;

        // ==== Addition ====
        Vector2 operator+(const Vector2& otherVector) const
        {
            return Vector2(this->x + otherVector.x, this->y + otherVector.y);
        }

        Vector2 operator+(const float value) const
        {
            return Vector2(this->x + value, this->y + value);
        }

        void operator+=(const Vector2& otherVector)
        {
            this->x += otherVector.x;
            this->y += otherVector.y;
        }

        // ==== Multiplication ====
        Vector2 operator*(const Vector2& otherVector) const
        {
            return Vector2(this->x * otherVector.x, this->y * otherVector.y);
        }

        void operator*=(const Vector2& otherVector)
        {
            this->x *= otherVector.x;
            this->y *= otherVector.y;
        }

        Vector2 operator*(const float value) const
        {
            return Vector2(this->x * value, this->y * value);
        }

        void operator*=(const float value)
        {
            this->x *= value;
            this->y *= value;
        }

        // ==== Substraction ====
        Vector2 operator-(const Vector2& otherVector) const
        {
            return Vector2(this->x - otherVector.x, this->y - otherVector.y);
        }

        Vector2 operator-(const float value) const
        {
            return Vector2(this->x - value, this->y - value);
        }

        void operator-=(const Vector2& otherVector)
        {
            this->x -= otherVector.x;
            this->y -= otherVector.y;
        }

        // ==== Division ====
        Vector2 operator/(const Vector2& otherVector) const
        {
            return Vector2(this->x / otherVector.x, this->y / otherVector.y);
        }

        Vector2 operator/(const float value) const
        {
            return Vector2(this->x / value, this->y / value);
        }

        void operator/=(const Vector2& otherVector)
        {
            this->x /= otherVector.x;
            this->y /= otherVector.y;
        }

        // Returns the length of ||V||. 
        float Length() const { return Utilities::SquareRoot(x * x + y * y); }

        // Returns the length squared.
        float LengthSquared() const { return x * x + y * y; }

        // Returns the distance between two vectors.
        static inline float Distance(const Vector2& vectorA, const Vector2& vectorB) { return (vectorB - vectorA).Length(); }

        // Returns the distance squared between two vectors.
        static inline float DistanceSquared(const Vector2& vectorA, const Vector2& vectorB) { return (vectorB - vectorA).LengthSquared(); }

        bool operator==(const Vector2& otherVector) const
        {
            return this->x == otherVector.x && this->y == otherVector.y;
        }

        bool operator!=(const Vector2& otherVector) const
        {
            return this->x != otherVector.x && this->y != otherVector.y;
        }

        // Returns a pointer to our vector data, needed for various APIs and libraries.
        const float* Data() const { return &x; }

        std::string ToString() const;
 
    public:
        float x = 0;
        float y = 0;
        
        static const Vector2 Zero;
        static const Vector2 One;
    };
}