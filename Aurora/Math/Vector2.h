#pragma once
#include <string>

namespace Aurora::Math
{
    class Vector2
    {
    public:
        Vector2(float x, float y) : m_X(x), m_Y(y)
        {

        }

    public:
        float m_X;
        float m_Y;
    };
}