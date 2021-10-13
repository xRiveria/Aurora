#pragma once

// Hide warnings which belong to Bullet.
#pragma warning(push, 0) 
#include <Bullet/LinearMath/btQuaternion.h>
#pragma warning(pop)

namespace Aurora
{
    inline btVector3 ToBulletVector3(const XMFLOAT3& vector)
    {
        return btVector3(vector.x, vector.y, vector.z);
    }

    inline XMFLOAT3 ToVector3(const btVector3& vector)
    {
        return XMFLOAT3(vector.getX(), vector.getY(), vector.getZ());
    }

    inline XMFLOAT4 ToVector4(const btVector3& vector)
    {
        return XMFLOAT4(vector.getX(), vector.getY(), vector.getZ(), 1.0f);
    }

    inline XMFLOAT4 ToVector4(const btVector4& vector)
    {
        return XMFLOAT4(vector.getX(), vector.getY(), vector.getZ(), vector.getW());
    }

    inline XMFLOAT3 operator*(XMFLOAT3& vector, float scalar)
    {
        return XMFLOAT3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
    }

    inline XMFLOAT3 operator*(const XMFLOAT3& vector, float scalar)
    {
        return XMFLOAT3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
    }

    inline XMFLOAT3 operator*(const XMFLOAT3& vector, XMFLOAT3 vectorMultiply)
    {
        return XMFLOAT3(vector.x * vectorMultiply.x, vector.y * vectorMultiply.y, vector.z * vectorMultiply.z);
    }

    inline XMFLOAT3 operator*(XMFLOAT3& vector, XMFLOAT3 vectorMultiply)
    {
        return XMFLOAT3(vector.x * vectorMultiply.x, vector.y * vectorMultiply.y, vector.z * vectorMultiply.z);
    }

    inline bool operator==(XMFLOAT3& setA, XMFLOAT3& setB)
    {
        if (setA.x == setB.x && setA.y == setB.y && setA.z == setB.z)
        {
            return true;
        }

        return false;
    }

    inline bool operator==(XMFLOAT4X4& setA, XMFLOAT4X4& setB)
    {
        return true;
    }

    inline bool operator==(const XMFLOAT4X4& setA, const XMFLOAT4X4& setB)
    {
        return true;
    }

    inline bool operator==(const XMFLOAT3& setA, const XMFLOAT3& setB)
    {
        if (setA.x == setB.x && setA.y == setB.y && setA.z == setB.z)
        {
            return true;
        }

        return false;
    }

    inline bool operator==(XMFLOAT4& setA, XMFLOAT4& setB)
    {
        if (setA.x == setB.x && setA.y == setB.y && setA.z == setB.z && setA.w == setB.w)
        {
            return true;
        }

        return false;
    }

    inline bool operator==(const XMFLOAT4& setA, const XMFLOAT4& setB)
    {
        if (setA.x == setB.x && setA.y == setB.y && setA.z == setB.z && setA.w == setB.w)
        {
            return true;
        }

        return false;
    }

    inline bool operator!=(XMFLOAT4& setA, XMFLOAT4& setB)
    {
        return !(setA == setB);
    }

    inline bool operator!=(const XMFLOAT4& setA, const XMFLOAT4& setB)
    {
        return !(setA == setB);
    }


    inline bool operator!=(XMFLOAT3& setA, XMFLOAT3& setB)
    {
        return !(setA == setB);
    }

    inline bool operator!=(const XMFLOAT3& setA, const XMFLOAT3& setB)
    {
        return !(setA == setB);
    }
}