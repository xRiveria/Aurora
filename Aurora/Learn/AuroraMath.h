#pragma once
#include "../Log/Log.h"
#include <math.h> 

// BOMDAS - Brackers, Order (Exponents), (Left to Right for the Rest) Division, Multiplication , Addition, Subtraction <--- Order of Operations. https://en.cppreference.com/w/cpp/language/operator_precedence
// If a large number is counting up and suddenly becomes negative (or vice versa), you have an integer overflow. This happens when your type (eg a 32 bit signed integer)
// runs out of range and cannot represent the number you're trying to represent. 
// 2 negatives makema positive!

namespace AuroraMath
{
    constexpr float PI = 3.14159265359;
    constexpr float TAU = 2 * PI;

    // Returns the number rounded down to its nearest whole number. 
    template<typename T>
    constexpr T Floor(T value)
    {
        return floor(value);
    }

    // Returns the number rounded up to its nearest whole number.
    template<typename T>
    constexpr T Ceil(T value)
    {
        return ceil(value);
    }

    // Returns value raised to power.
    template<typename T>
    constexpr T Pow(T value, float power)
    {
        return pow(value, power);
    }

    // Returns the square root of value.
    template<typename T>
    constexpr T Sqrt(T value)
    {
        return sqrt(value);
    }

    /*  Modulo Alternative
     
        If rand() gives us 64 and we're trying to find out which player starts first using a coin setting (Heads & Tails = 2 Players / 2 Outcomes)
        1) 65 / 2 = 32.5
        2) Cast 32 away.
        3) Take 0.5 x 2 = 1. Player 2 starts first.

        Shortcut of all the above?
        1) 65 % 2 = 1. Player 2 starts first.
    */
    int HeadOrTail()
    {
        if (rand() % 2 == 0)
        {
            AURORA_INFO(Aurora::LogLayer::Engine, "Flipped Heads!");
            return 0;
        }
        else
        {
            AURORA_INFO(Aurora::LogLayer::Engine, "Flipped Tails!");
            return 1;
        }
    }

    static float Rad2Deg(float radians)
    {
        return radians * 180.0f / PI; // Or radians * 360.0f / TAU (2PI).
    }

    static float Deg2Rad(float degrees)
    {   
        return degrees * PI / 180.0f; // Or degrees * TAU / 360.0f.
    }
}
 