#pragma once
#include <numeric>

namespace Aurora::Math::Utilities
{
    template <typename T>
    constexpr T SquareRoot(T x) { return sqrt(x); }

    // Check for equality but allow for a small margin of error.
    template <typename T>
    constexpr bool Equals(T argumentA, T argumentB, T marginOfError = std::numeric_limits<T>::epsilon()) { return argumentA + marginOfError >= argumentB && argumentA - marginOfError <= argumentB; }

    // Returns the absolute value.
    template <typename T>
    constexpr T Absolute(T value) { return value >= static_cast<T>(0) ? value : -value; }
}