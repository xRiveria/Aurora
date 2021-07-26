#pragma once
#include <numeric>

namespace Aurora::Math::Utilities
{
    constexpr float PI = 3.14159265359f;
    constexpr float _DegreesToRadians = PI / 180.0f;
    constexpr float _RadiansToDegrees = 180.0f / PI;

    constexpr float DegreesToRadians(const float degrees) { return degrees * _DegreesToRadians; }
    constexpr float RadiansToDegrees(const float radians) { return radians * _RadiansToDegrees; }

    template <typename T>
    constexpr T SquareRoot(T x) { return sqrt(x); }

    // Check for equality but allow for a small margin of error.
    template <typename T>
    constexpr bool Equals(T argumentA, T argumentB, T marginOfError = std::numeric_limits<T>::epsilon()) { return argumentA + marginOfError >= argumentB && argumentA - marginOfError <= argumentB; }

    // Returns the absolute value.
    template <typename T>
    constexpr T Absolute(T value) { return value >= static_cast<T>(0) ? value : -value; }

    // Return the sign of our number.
    template <typename T>
    constexpr int Sign(T x) { return (static_cast<T>(0) < x) - (x < static_cast<T>(0)); }

    template <typename T>
    constexpr T Minimum(T a, T b) { return a < b ? a : b; }

    template <typename T>
    constexpr T Maximum(T a, T b) { return a > b ? a : b; }
}