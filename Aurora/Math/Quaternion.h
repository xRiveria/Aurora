#pragma once

namespace Aurora::Math
{
    class Quaternion
    {
    public:
        Quaternion()
        {
            x = 0;
            y = 0;
            z = 0;
            w = 0;
        }

        Quaternion(float x, float y, float z, float w)
        {

        }

        static inline Quaternion Multiply(const Quaternion& quaternionA, const Quaternion& quaternionB)
        {
            const float aX = quaternionA.x;
            const float aY = quaternionA.y;
            const float aZ = quaternionA.z;
            const float aW = quaternionA.w;
        }

        Quaternion operator*(const Quaternion& otherQuaternion) const
        {
            return Multiply(*this, otherQuaternion);
        }

        static const Quaternion Identity;

    public:
        float x = 0;
        float y = 0;
        float z = 0;
        float w = 1;
    };
}